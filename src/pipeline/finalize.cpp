#include <pipeline.hpp>
#include <dependency_source.hpp>
#include <build_cache.hpp>

namespace rakpak::pipeline
{
    /* TODO: Clean this */
    static void export_library(PipelineContext& pipeline_context)
    {
        BuildContext& build_context = pipeline_context.build_context;
        BuildOutput& build_output = build_context.build_output;
        AppContext& app_context = pipeline_context.app_context;
        auto& logger = app_context.logger;
        auto& target = build_context.target;
        Module _module = target.to_module();
        _module.usage_requirements.link_flags
            .push_back("-L" + fs::absolute(build_output.binary.directory).string());
        for (const auto& dep : target.dependencies)
        {
            if (dep.visibility == project::Visibility::Private)
                continue;
            auto _dependency_module = PackageManager::instance().get_module(dep.from, dep.name);
            if (_dependency_module != nullptr)
            {
                for (const auto& path : _dependency_module->usage_requirements.include_paths)
                    _module.usage_requirements.include_paths.push_back(fs::absolute(path));
                for (const auto& flag : _dependency_module->usage_requirements.link_flags)
                    _module.usage_requirements.link_flags.push_back(flag);
                for (const auto& flag : _dependency_module->usage_requirements.build_flags)
                    _module.usage_requirements.build_flags.push_back(flag);
            }
        }
        build_context.module_provider.add_module(_module);
        for (auto& [name, targets] : build_context.provides)
        {
            if (std::find(targets.begin(), targets.end(), target.metadata.name) != targets.end())
            {
                logger->log_debug("Exporing as module: '{}'", name);
                _module.name = name;
                PackageManager::instance().add_module(build_context.project_name, _module);
            }
        }
    }

    void finalize(PipelineContext& pipeline_context)
    {
        auto& app_context = pipeline_context.app_context;
        auto expected_state = pipeline::get_expected_state(pipeline::PipelineState::Finalized);
        if (!pipeline::validate_pipeline_state(pipeline_context, expected_state))
            exit(1);
        pipeline_context.state = pipeline::PipelineState::Finalized;
        auto& build_context = pipeline_context.build_context;
        app_context.logger->log_success("Build succeeded");
        if (build_context.target.metadata.type == project::BuildTarget::Type::Library)
            export_library(pipeline_context);

        if (!pipeline_context.prefix_path.empty())
        {
            fs::path source = build_context.build_output.binary.full_path();
            fs::path destination;
            if (build_context.target.metadata.type == project::BuildTarget::Type::Executable)
                destination = pipeline_context.prefix_path / "bin" / build_context.build_output.binary.name;
            else if (build_context.target.metadata.type == project::BuildTarget::Type::Library)
                destination = pipeline_context.prefix_path / "lib" / build_context.build_output.binary.name;
            if (fs::exists(destination))
            {
                auto source_last_write = fs::last_write_time(source);
                auto destination_last_write = fs::last_write_time(destination);
                if (destination_last_write >= source_last_write)
                    return;
                // Remove stale
                fs::remove(destination);
            }

            fs::copy_file(source, destination);
        }
    }
}