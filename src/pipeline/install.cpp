#include <pipeline.hpp>

namespace rakpak::pipeline
{
    /*
    static void get_install_target_root_path(PipelineContext& pipeline_context)
    {
        AppContext& app_context = pipeline_context.app_context;
        InstallTarget& install_target = pipeline_context.install_target;
        const fs::path home_path = fs::path(getenv("HOME"));
        switch (app_context.install_scope)
        {
            case InstallScope::Local:
                install_target.root_path = fs::current_path() / "local";
            break;
            case InstallScope::User:
                install_target.root_path = home_path / ".local";
            break;
            case InstallScope::System:
                install_target.root_path = "/usr/local";
            break;
        }
    }

    static void get_install_target_bin_path(PipelineContext& pipeline_context)
    {
        InstallTarget& install_target = pipeline_context.install_target;
        install_target.bin_path = install_target.root_path / "bin";
    }

    static void get_install_target_lib_path(PipelineContext& pipeline_context)
    {
        InstallTarget& install_target = pipeline_context.install_target;
        install_target.lib_path = install_target.root_path / "lib";
    }

    static void get_install_target_include_path(PipelineContext& pipeline_context)
    {
        InstallTarget& install_target = pipeline_context.install_target;
        std::string project_name = pipeline_context.build_context.project.name;
        install_target.include_path = install_target.root_path / "include" / project_name;
    }

    static void get_install_target_assets_path(PipelineContext& pipeline_context)
    {
        InstallTarget& install_target = pipeline_context.install_target;
        std::string project_name = pipeline_context.build_context.project.name;
        install_target.asset_path = install_target.root_path / "share" / project_name;
    }

    void initialize_install_target(PipelineContext& pipeline_context)
    {        
        BuildContext& builder_context = pipeline_context.build_context;
        AppContext& app_context = pipeline_context.app_context;
        InstallTarget& install_target = pipeline_context.install_target;
        get_install_target_root_path(pipeline_context);
        get_install_target_bin_path(pipeline_context);
        get_install_target_lib_path(pipeline_context);
        get_install_target_include_path(pipeline_context);
        get_install_target_assets_path(pipeline_context);
    }

    static void install_binary_to_path(
        AppContext& app_context, 
        std::string_view binary_name,
        const fs::path& binary_path,
        const fs::path& install_path)
    {
        if (!fs::exists(install_path))
        {
            app_context.logger->log_info("Install directory '{}' didn't exist creating", install_path.string());
            fs::create_directories(install_path);
        }
        const fs::path file_path = install_path / binary_name;
        if (fs::exists(file_path))
            fs::remove(file_path);
        app_context.logger->log_info("Installing '{}' to '{}'", binary_name, install_path.string());
        fs::copy_file(binary_path, file_path, fs::copy_options::update_existing);
    }

    static void install_exported_headers(
        AppContext& app_context, 
        const BuildContext& builder_context,
        const fs::path& install_path)
    {
        auto& project = builder_context.project;
        auto& exports = builder_context.exports;
        
        fs::create_directories(install_path);
        for (auto& exported_headers_path : exports.include_directories)
        {
            app_context.logger->log_info("Installing headers to '{}'", install_path.string());
            fs::copy(project.directory / exported_headers_path, install_path, fs::copy_options::recursive);
        }
    }
    static void install_package_headers(
        AppContext& app_context, 
        const PackageInfo& package,
        const fs::path& install_path)
    {
        fs::create_directories(install_path);
        for (auto& exported_headers_path : package.usage_requirements.include_paths)
        {
            app_context.logger->log_info("Installing headers to '{}'", install_path.string());
            fs::copy(exported_headers_path, install_path, fs::copy_options::recursive);
        }
    }

    void install(PipelineContext& pipeline_context)
    {
        AppContext& app_context = pipeline_context.app_context;
        const BuildContext& builder_context = pipeline_context.build_context;
        const InstallTarget& install_target = pipeline_context.install_target;
        const BuildOutput::Binary& binary = builder_context.build_output.binary;

        switch (builder_context.project.type)
        {
            case ProjectType::Executable:
                install_binary_to_path(app_context, binary.name, binary.full_path(), install_target.bin_path);
            break;
            case ProjectType::Library:
                install_binary_to_path(app_context, binary.name, binary.full_path(), install_target.lib_path);
                install_exported_headers(app_context, builder_context, install_target.include_path);
            break;
            case ProjectType::HeaderOnly:
                install_exported_headers(app_context, builder_context, install_target.include_path);
            break;
        }
        for (const auto& dep : builder_context.build_output.package.usage_requirements.dependencies)
        {
            auto filename = dep.build_path.filename().string();
            install_binary_to_path(app_context, filename, dep.build_path, install_target.lib_path);
            install_package_headers(app_context, dep, install_target.include_path);
        }
    }
    */
}