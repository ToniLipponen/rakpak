#include <build/pipeline.hpp>
#include <pkg-config.hpp>
#include <dependency_source.hpp>
#include <build_cache.hpp>

namespace rakpak::pipeline
{
    using namespace project;

    static void initialize_build_output(AppContext& app_context, BuildContext& ctx) 
    {
        auto& logger = app_context.logger;

        //auto& package = ctx.build_output.package;
        auto& binary = ctx.build_output.binary;
        auto& object = ctx.build_output.object;

        logger->log_info("Initializing build output in '{}'", ctx.project_root.string());

        binary.root_directory = ctx.project_root / "bin";
        binary.directory = binary.root_directory / ctx.build_profile.name;
        object.root_directory = ctx.project_root / "obj";
        object.directory = object.root_directory / ctx.build_profile.name;

        if (ctx.target.metadata.output_name.empty())
            ctx.target.metadata.output_name = ctx.target.metadata.name;

        if (ctx.target.metadata.type == BuildTarget::Type::Executable)
            binary.name = ctx.target.metadata.output_name;
        else
        {
            if (ctx.target.metadata.variant == BuildTarget::Variant::Shared)
                binary.name = "lib" + ctx.target.metadata.output_name + ".so";
            else
                binary.name = "lib" + ctx.target.metadata.output_name + ".a";
        }
        if (!fs::exists(binary.directory))
        {
            logger->log_info("Creating directory '{}'", binary.directory.string());
            fs::create_directories(binary.directory);
        }
        if (!fs::exists(object.directory))
        {
            logger->log_info("Creating directory '{}'", object.directory.string());
            fs::create_directories(object.directory);
        }
    }

    void initialize(PipelineContext& pipeline_context, const Project& project)
    {
        AppContext& app_context = pipeline_context.app_context;
        auto& logger = app_context.logger;
        auto scope = logger->scope("Pipeline");

        logger->log_info(
            "Initializing build context for {}:{}", 
            project.metadata.name,
            app_context.build_options.profile 
        );

        auto& ctx = pipeline_context.build_context = {};
        ctx.project_name = project.metadata.name;
        ctx.provides = project.provides;
        auto& queue = ctx.build_queue;

        std::function<void(const BuildTarget&)> queue_next = [&](const BuildTarget& target)
        {
            for (auto& dep : target.dependencies)
                if (dep.from == project.metadata.name)
                    queue_next(project.build_targets.at(dep.name));

            auto it = std::find_if(queue.begin(), queue.end(), [&target](const BuildTarget& other)
            {
                return other.metadata.name == target.metadata.name;
            });

            if (it != queue.end())
                return;

            queue.push_back(target);
        };

        for (const auto& [_, build_target] : project.build_targets)
            queue_next(build_target);
        auto build_profiles_it = project.build_profiles.find(app_context.build_options.profile);
        if (build_profiles_it != project.build_profiles.end())
            ctx.build_profile = build_profiles_it->second;
        else
            ctx.build_profile.name = "debug";
        pipeline_context.build_cache.reset();
    }

    TU get_tu_info(const fs::path& src_file_path, const BuildOutput::Object& object_output)
    {
        const fs::path src_normalized = src_file_path.lexically_normal();
        const fs::path src_dir = fs::path(src_normalized).remove_filename();
        const fs::path object_file_name = src_normalized.filename().replace_extension(".o");
        const fs::path dep_file_name = src_normalized.filename().replace_extension(".d");
        const fs::path output_dir = object_output.directory / src_dir; 
        if (!fs::exists(output_dir))
            fs::create_directories(output_dir);
        return TU
        {
            src_normalized,
            output_dir / object_file_name,
            output_dir / dep_file_name
        };
    }

    void advance_build_queue(PipelineContext& pipeline_context)
    {
        AppContext& app_context = pipeline_context.app_context;
        auto& logger = app_context.logger;

        auto& ctx = pipeline_context.build_context;
        // Clear
        ctx.linker_args.clear();
        ctx.build_output = {};
        //
        auto target = ctx.target = ctx.build_queue.front();
        auto scope = logger->scope(target.metadata.name);
        ctx.build_queue.pop_front();
        initialize_build_output(app_context, ctx);

        ctx.linker_args.push_back_range(target.link_flags.begin(), target.link_flags.end());
        //std::vector<const Module*> modules;
        UniqueVector<const Module*> modules;
        for (const auto& dependency : target.dependencies)
        {
            auto _module = PackageManager::instance().get_module(dependency.from, dependency.name);
            if (_module == nullptr)
            {
                if (dependency.from == "pkg-config")
                {
                    auto pkg_info = pkg_config::get_pkg(dependency.name);
                    Module new_pkg;
                    new_pkg.name = dependency.name;
                    new_pkg.usage_requirements.include_paths = pkg_info.include_paths;
                    new_pkg.usage_requirements.link_flags = pkg_info.libs;
                    new_pkg.usage_requirements.build_flags = pkg_info.cflags;
                    new_pkg.linkage = BuildTarget::Variant::Shared;
                    PackageManager::instance().add_module(dependency.from, new_pkg);
                }
                else if (dependency.from == "system")
                {
                    Module new_pkg;
                    new_pkg.name = dependency.name;
                    new_pkg.usage_requirements.link_flags.push_back("-l" + dependency.name);
                    PackageManager::instance().add_module(dependency.from, new_pkg);
                }
            }
            _module = PackageManager::instance().get_module(dependency.from, dependency.name);
            if (_module == nullptr)
                _module = ctx.module_provider.get_module(dependency.name);
            if (_module != nullptr)
                modules.push_back(_module);
                //satisfy_package_usage_requirements(pipeline_context, pkg.value());
            else
            {
                logger->log_error("Failed to find module '{}' from source '{}'", dependency.name, dependency.from);
                exit(1);
            }
        }
        ctx.compiler_args.clear();
        ctx.compiler_args.push_back("-MMD");
        if (ctx.target.metadata.type == BuildTarget::Type::Library
            && ctx.target.metadata.variant == BuildTarget::Variant::Shared)
            ctx.compiler_args.push_back("-fPIC");
        
        // Build flags
        ctx.compiler_args.push_back_range(target.build_flags.begin(), target.build_flags.end());
        ctx.compiler_args.push_back_range(ctx.build_profile.build_flags.begin(), ctx.build_profile.build_flags.end());
        for (auto mod : modules.get_vector())
        {
            auto& ur = mod->usage_requirements;
            ctx.compiler_args.push_back_range(ur.build_flags.begin(), ur.build_flags.end());
            ctx.linker_args.push_back_range(ur.link_flags.begin(), ur.link_flags.end());
        }
        for (const auto& define : target.defines)
            ctx.compiler_args.push_back(define.to_string());
        for (const auto& define : ctx.build_profile.defines)
            ctx.compiler_args.push_back(define.to_string());
        //
        // Include paths
        for (const auto& path : target.include_directories)
            ctx.compiler_args.push_back("-I" + path.string());
        for (auto mod : modules.get_vector())
        {
            auto& ur = mod->usage_requirements;
            for (auto& path : ur.include_paths)
                ctx.compiler_args.push_back("-I" + path.string());
        }

        for (const auto& path : target.source_files)
        {
            TU tu = get_tu_info(path, ctx.build_output.object);
            if (pipeline_context.build_cache.needs_rebuild(tu.source_path, tu.object_path))
            {
                logger->log_info("Detected changes '{}' rebuilding", path.lexically_normal().string());
                //ctx.source_files.push_back(path);
                ctx.tu_queue.push_back(std::move(tu));
            }
            else
            {
                logger->log_info("Up to date, skipping '{}'", path.lexically_normal().string());
                ctx.build_output.object.objects.push_back(std::move(tu.object_path));
            }
        }
        ctx.linker_args.push_back("-Wl,-rpath,$ORIGIN/../lib/");
        pipeline_context.state = PipelineState::Initialized;
    }
}