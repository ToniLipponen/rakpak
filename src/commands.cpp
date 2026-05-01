#include <commands.hpp>
#include <pipeline.hpp>
#include <process.hpp>
#include <utils/string.hpp>
#include <thread>

namespace rakpak::cli
{
    using namespace rakpak::project;
    static void project_build(pipeline::PipelineContext& pipeline_context, const Project& project)
    {
        AppContext& app_context = pipeline_context.app_context;
        pipeline::initialize(pipeline_context, project);
        while (!pipeline_context.build_context.build_queue.empty())
        {
            pipeline::advance_build_queue(pipeline_context);
            pipeline::compile(pipeline_context);
            pipeline::link(pipeline_context);
            pipeline::finalize(pipeline_context);
        }
    }

    static void create_local_prefix(const fs::path& path)
    {
        fs::create_directories(path / "bin");
        fs::create_directories(path / "lib");
        fs::create_directories(path / "include");
        fs::create_directories(path / "share");
    }

    void build(AppContext& app_context)
    {
        Project project = project::parse_from_directory(app_context, ".");
        fs::path root_path = fs::current_path();

        for (const auto& [_, path] : project.subprojects)
        {
            fs::current_path(path);
            Project subproject = project::parse_from_directory(app_context, ".");
            pipeline::PipelineContext pipeline_context = { app_context };
            project_build(pipeline_context, subproject);
        }
        fs::current_path(root_path);
        pipeline::PipelineContext pipeline_context = { app_context };
        project_build(pipeline_context, project);
    }

    void run(AppContext& app_context)
    {
        Project project = project::parse_from_directory(app_context, ".");
        fs::path root_path = fs::current_path();

        fs::path prefix_path = root_path / ".run";
        create_local_prefix(prefix_path);

        for (const auto& [_, path] : project.subprojects)
        {
            fs::current_path(path);
            Project subproject = project::parse_from_directory(app_context, ".");
            pipeline::PipelineContext pipeline_context = { app_context };
            pipeline_context.prefix_path = prefix_path;
            project_build(pipeline_context, subproject);
        }

        fs::current_path(root_path);
        pipeline::PipelineContext pipeline_context = { app_context };
        pipeline_context.prefix_path = prefix_path;
        project_build(pipeline_context, project);

        app_context.logger->log_info("Running run command");

        // The builder context left in pipeline_context should be for the top level project
        // Which should be executable
        BuildContext& builder_context = pipeline_context.build_context;
        std::string command = "./" + builder_context.build_output.binary.name;
        // TODO: Get application parameters from command
        // Meaning if the command is for example:
        // rakpak run -- abc 123
        // abc and 123 should be passed into the executable when invoked
        process::invoke(prefix_path / "bin", command);
    }

    static void project_clean(AppContext& app_context, const Project& project)
    {
        auto& logger = app_context.logger;
        logger->log_info("Cleaning {}", project.metadata.directory.string());
        fs::path bin_directory = project.metadata.directory / "bin";
        fs::path obj_directory = project.metadata.directory / "obj";

        logger->log_debug("Removing directory '{}'", bin_directory.string());
        fs::remove_all(bin_directory);

        logger->log_debug("Removing directory '{}'", obj_directory.string());
        fs::remove_all(obj_directory);

        if (app_context.clean_options.purge)
        {
            fs::remove_all(project.metadata.directory / ".run");
            fs::remove_all(project.metadata.directory / ".rakpak");
        }

        if (app_context.clean_options.recursive)
        {
            auto root_path = fs::current_path();
            for (const auto& [_, subproject_path] : project.subprojects)
            {
                fs::current_path(subproject_path);
                Project subproject = project::parse_from_directory(app_context, ".");
                project_clean(app_context, subproject);
            }
            fs::current_path(root_path);
        }
    }

    void clean(AppContext& app_context)
    {
        app_context.logger->log_info("Running clean command");
        Project project = project::parse_from_directory(app_context, ".");
        project_clean(app_context, project);
    }

    void reset(AppContext& app_context)
    {
        app_context.logger->log_info("Running reset command");
        clean(app_context);
        if (fs::exists(".rakpak"))
            fs::remove_all(".rakpak");
    }

    void install(AppContext& app_context)
    {
        app_context.logger->log_info("Running build command");
        Project project = project::parse_from_directory(app_context, ".");
        fs::path root_path = fs::current_path();

        fs::path prefix_path = root_path / "local";
        if (app_context.install_scope == InstallScope::System)
            prefix_path = "/usr/local";
        else if (app_context.install_scope == InstallScope::User)
            prefix_path = fs::path(getenv("HOME")) / ".local";

        create_local_prefix(prefix_path);

        for (const auto& [_, path] : project.subprojects)
        {
            fs::current_path(path);
            Project subproject = project::parse_from_directory(app_context, ".");
            pipeline::PipelineContext pipeline_context = { app_context };
            pipeline_context.prefix_path = prefix_path;
            project_build(pipeline_context, subproject);
        }

        fs::current_path(root_path);
        pipeline::PipelineContext pipeline_context = { app_context };
        pipeline_context.prefix_path = prefix_path;
        project_build(pipeline_context, project);
    }

    void validate(AppContext& app_context)
    {
        // Validate project, and it's subprojects if --recursive,-r is used
    }

    void export_compile_commands(AppContext& app_context)
    {
        // Idea, export all necessary shit for IDEs to not error out on include path related stuff
        // compile_commands.json
    }

    void generate_from_template(
        AppContext& app_context, 
        std::string_view name, 
        std::optional<std::string_view> project_name)
    {
    }

    void describe_feature(AppContext& app_context)
    {
    }

    void generate_pkg_config(AppContext& app_context)
    {
    }
}