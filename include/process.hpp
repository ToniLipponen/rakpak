#pragma once
#include <string>
#include <vector>
#include <future>
#include <filesystem>
#include <optional>

namespace fs = std::filesystem;

namespace rakpak::process
{
    struct ProcessResult
    {
        int exit_code = 0;
        struct Output 
        {
            std::string standard;
            std::string error;
        } output;
    };

    namespace impl
    {
        int invoke(const char** argv, std::optional<fs::path> working_dir = std::nullopt) noexcept;
        //int invoke(const fs::path& working_dir, const char** argv) noexcept;
        ProcessResult invoke_capture(const char** argv) noexcept;
    }

    int invoke(std::string_view exec);
    int invoke(const fs::path& working_dir, std::string_view exec);

    template<class Range>
    int invoke(std::string_view exec, const Range& args);
   
    ProcessResult invoke_capture(std::string_view exec);

    template<class Range>
    ProcessResult invoke_capture(std::string_view exec, const Range& args);

    std::future<int> invoke_async(std::string exec);

    template<class Range>
    std::future<int> invoke_async(std::string exec, Range args);
   
    std::future<ProcessResult> invoke_capture_async(std::string exec);

    template<class Range>
    std::future<ProcessResult> invoke_capture_async(std::string exec, Range args);
}

#include <process.tpp>