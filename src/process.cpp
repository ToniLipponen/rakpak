#include <process.hpp>
#include <utils/string.hpp>
#include <unistd.h>
#include <sys/wait.h>
#include <poll.h>
#include <cerrno>
#include <cassert>

#if 0
    #define DEBUG_DUMP_INVOKE_ARGV(argv) dump_invoke_argv(argv)
#else
    #define DEBUG_DUMP_INVOKE_ARGV(argv) 
#endif

namespace rakpak::process::impl
{
    static void dump_invoke_argv(const char** argv)
    {
        for (int i = 0;; i++)
        {
            if (argv[i] == nullptr)
                break;
            printf("'%s'\n", argv[i]);
        }
    }

    int invoke(const char** argv, std::optional<fs::path> working_dir) noexcept
    {
        DEBUG_DUMP_INVOKE_ARGV(argv);
        pid_t pid = fork();
        if (pid == 0)
        {
            if (working_dir.has_value())
            {
                auto path = working_dir.value();
                chdir(path.c_str());
            }
            execvp(argv[0], const_cast<char* const*>(argv));
            _exit(127);
        }
        int status;
        waitpid(pid, &status, 0);
        return status;
    }

    ProcessResult invoke_capture(const char** argv) noexcept
    {
        DEBUG_DUMP_INVOKE_ARGV(argv);
        int stdout_pipe[2]{};
        int stderr_pipe[2]{};

        pipe(stdout_pipe);
        pipe(stderr_pipe);

        pid_t pid = fork();

        if (pid == 0)
        {
            dup2(stdout_pipe[1], STDOUT_FILENO);
            dup2(stderr_pipe[1], STDERR_FILENO);

            close(stdout_pipe[0]); close(stdout_pipe[1]);
            close(stderr_pipe[0]); close(stderr_pipe[1]);

            execvp(argv[0], const_cast<char* const*>(argv));
            _exit(127);
        }

        close(stdout_pipe[1]);
        close(stderr_pipe[1]);

        ProcessResult captured;

        struct pollfd poll_fds[2];
        poll_fds[0] = pollfd { stdout_pipe[0], POLLIN, 0 };
        poll_fds[1] = pollfd { stderr_pipe[0], POLLIN, 0 };

        char buffer[1024];
        int open_fds = 2;

        auto close_pollfd = [&](int i)
        {
            close(poll_fds[i].fd);
            poll_fds[i].fd = -1;
            open_fds--;
        };
        while (open_fds > 0)
        {
            int ret = poll(poll_fds, 2, -1);
            if (ret < 0)
            {
                if (errno == EINTR)
                    continue;
                else
                    break;
            }
            for (int i = 0; i < 2; i++)
            {
                if (poll_fds[i].fd == -1)
                    continue;
                if (poll_fds[i].revents & POLLHUP)
                    close_pollfd(i);
                if (poll_fds[i].revents & POLLIN)
                {
                    ssize_t bytes = read(poll_fds[i].fd, buffer, sizeof(buffer));
                    if (bytes > 0)
                    {
                        if (i == 0) // stdin
                            captured.output.standard.append(buffer, bytes);
                        else // stderr
                            captured.output.error.append(buffer, bytes);
                    }
                    else if (bytes == 0 || errno != EINTR)
                        close_pollfd(i);
                }
            }
        }
        int status = 0;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
            captured.exit_code = WEXITSTATUS(status);
        else if (WIFSIGNALED(status))
            captured.exit_code = WTERMSIG(status) + 128;
        return captured;
    }
}

namespace rakpak::process
{
    int invoke(std::string_view command)
    {
        std::vector<const char*> argv;
        argv.push_back(command.data());
        argv.push_back(nullptr);
        return impl::invoke(argv.data());
    }

    int invoke(const fs::path& working_dir, std::string_view command)
    {
        std::vector<const char*> argv;
        argv.push_back(command.data());
        argv.push_back(nullptr);
        return impl::invoke(argv.data(), working_dir);
    }

    ProcessResult invoke_capture(std::string_view command)
    {
        return invoke_capture(command, std::array<std::string_view, 0>{});
    }

    std::future<int> invoke_async(std::string exec)
    {
        return std::async(
            std::launch::async,
            [command = std::move(exec)]{ return invoke(command); } 
        );
    }

    std::future<ProcessResult> invoke_capture_async(std::string exec)
    {
        return std::async(
            std::launch::async,
            [command = std::move(exec)]{ return invoke_capture(command); } 
        );
    }
}