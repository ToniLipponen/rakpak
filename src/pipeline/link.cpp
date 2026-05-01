#include <pipeline.hpp>
#include <process.hpp>
#include <project.hpp>

/*
TODO:
    Skip linking if it's not necessary.
*/
namespace rakpak::pipeline
{
    static void append_linker_invoke_args(
        std::vector<std::string_view>& invoke_args,
        const std::vector<std::string>& link_flags, 
        const std::vector<fs::path>& object_files)
    {
        for (const auto& object_file : object_files)
            invoke_args.push_back(object_file.c_str());
        for (const auto& flag : link_flags)
            invoke_args.push_back(flag);
    }

    static int link_static_library(const BuildContext& ctx)
    {
        const auto& binary = ctx.build_output.binary;
        const auto& object = ctx.build_output.object;
        const std::string full_path = binary.full_path();
        std::vector<std::string_view> invoke_args;
        invoke_args.push_back("rcs");
        invoke_args.push_back(full_path);
        for (const auto& obj : object.objects)
            invoke_args.push_back(obj.c_str());
        return process::invoke("ar", invoke_args);
    }

    static int link_shared_library(const BuildContext& ctx)
    {
        const auto& binary = ctx.build_output.binary;
        const auto& object = ctx.build_output.object;
        const std::string full_path = binary.full_path();
        const std::string soname_flag = "-Wl,-soname," + binary.name;

        std::vector<std::string_view> invoke_args;
        invoke_args.push_back("-shared");
        invoke_args.push_back(soname_flag);
        append_linker_invoke_args(invoke_args, ctx.linker_args.get_vector(), object.objects);
        invoke_args.push_back("-o");
        invoke_args.push_back(full_path);
        return process::invoke("c++", invoke_args);
    }

    static int link_executable(const BuildContext& ctx)
    {
        const auto& binary = ctx.build_output.binary;
        const auto& object = ctx.build_output.object;
        const std::string full_path = binary.full_path();
        std::vector<std::string_view> invoke_args;
        append_linker_invoke_args(invoke_args, ctx.linker_args.get_vector(), object.objects);
        invoke_args.push_back("-o");
        invoke_args.push_back(full_path);
        return process::invoke("c++", invoke_args);
    }

    void link(PipelineContext& pipeline_context)
    {
        auto& ctx = pipeline_context.build_context;
        auto& app_context = pipeline_context.app_context;
        auto& logger = app_context.logger;
        auto& binary = ctx.build_output.binary;
        auto& object = ctx.build_output.object;
        auto scope = logger->scope("Link");
        auto expected_state = get_expected_state(PipelineState::Linked);
        if (!validate_pipeline_state(pipeline_context, expected_state))
            return;
        logger->log_info("Linking object files to '{}'", binary.full_path().string());
        auto& build_target = ctx.target;
        int status = 0;
        switch (build_target.metadata.type)
        {
            case project::BuildTarget::Type::Library:
            {
                if (build_target.metadata.variant == project::BuildTarget::Variant::Static)
                    status = link_static_library(ctx);
                else
                    status = link_shared_library(ctx);
            }
            break;
            case project::BuildTarget::Type::Executable:
                status = link_executable(ctx);
            break;
        }
        if (status != 0)
        {
            logger->log_error("Failed to link");
            exit(1);
        }
        pipeline_context.state = PipelineState::Linked;
    }
}