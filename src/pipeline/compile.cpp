#include <pipeline.hpp>
#include <process.hpp>
#include <utils/string.hpp>
#include <build_cache.hpp>
#include <thread>
#include <atomic>
#include <iostream>
#include <chrono>

namespace rakpak::pipeline
{
    static int compile_object(PipelineContext& pipeline_context, const TU& tu, BuildOutput::Object& object_output)
    {
        auto& ctx = pipeline_context.build_context;
        auto& app_context = pipeline_context.app_context;
        LoggerBase* logger = app_context.logger.get();

        auto& build_output = ctx.build_output;
        auto object_dir = build_output.object.directory;

        thread_local static std::vector<std::string_view> invoke_args;
        invoke_args.clear();
        for (auto& arg : ctx.compiler_args.get_vector())
            invoke_args.push_back(arg);
        invoke_args.push_back("-MF");
        invoke_args.push_back(tu.dep_path.c_str());
        invoke_args.push_back("-c");
        invoke_args.push_back(tu.source_path.c_str());
        invoke_args.push_back("-o");
        invoke_args.push_back(tu.object_path.c_str());

        logger->log_info("'{}'", tu.source_path.c_str());
        process::ProcessResult result = process::invoke_capture("c++", invoke_args);
        if (result.exit_code != 0)
        {
            auto lines = utils::string::split_sv(result.output.error, "\n");
            for (auto line : lines)
            {
                if (utils::string::contains(line, "error:"))
                {
                    auto error_parts = utils::string::split_sv(line, "error:", utils::string::SplitRule::MatchFull);
                    logger->log_error("{} {}", error_parts[0], error_parts[1]);
                }
                else if (utils::string::contains(line, "warning:"))
                {
                    auto error_parts = utils::string::split_sv(line, "warning:", utils::string::SplitRule::MatchFull);
                    logger->log_warning("{} {}", error_parts[0], error_parts[1]);
                }
            }
        }
        else
            pipeline_context.build_cache.update_cache(tu.source_path, tu.dep_path);
        object_output.objects.push_back(tu.object_path);
        return result.exit_code;
    }

    void compile(PipelineContext& pipeline_context)
    {
        auto& ctx = pipeline_context.build_context;
        auto& app_context = pipeline_context.app_context;
        auto& logger = app_context.logger;
        auto expected_state = get_expected_state(PipelineState::Compiled);
        if (!validate_pipeline_state(pipeline_context, expected_state))
            return;
        auto scope = logger->scope("Compile");
        size_t max_threads = app_context.build_options.jobs;
        if (max_threads == 0)
            max_threads = std::thread::hardware_concurrency();
        std::atomic<size_t> index{0};
        std::atomic<bool> error = false;
        std::vector<std::thread> threads;
        threads.reserve(max_threads);

        std::vector<BuildOutput::Object> per_thread_build_output;
        per_thread_build_output.reserve(max_threads);

        const size_t files_count = ctx.tu_queue.size();

        for (size_t i = 0; i < max_threads; ++i)
        {
            per_thread_build_output.emplace_back();
            threads.emplace_back([&, i]
            {
                BuildOutput::Object& object_output = per_thread_build_output[i];
                auto worker_scope = logger->scope("Compile");
                while (error.load() == false)
                {
                    size_t n = index.fetch_add(1);
                    if (n >= files_count)
                        break;
                    const auto& tu = ctx.tu_queue.at(n);
                    int status = compile_object(pipeline_context, tu, object_output);
                    if (status != 0)
                        error.store(true);
                }
            });
        }
        for (auto& thread : threads)
            thread.join();
        if (error.load())
        {
            logger->log_error("Failed to compile");
            return;
        }
        for (auto& object_output : per_thread_build_output)
            for (auto& object_file : object_output.objects)
                ctx.build_output.object.objects.push_back(std::move(object_file));
        pipeline_context.state = PipelineState::Compiled;
    }
}