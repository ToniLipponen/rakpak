#pragma once
#include <build_context.hpp>
#include <app_context.hpp>
#include <install_target.hpp>
#include <build_cache.hpp>
#include <optional>

namespace rakpak::pipeline
{
    enum class PipelineState
    {
        Uninitialized,
        Initialized,
        Compiled,
        Linked,
        Finalized,
    };

    struct PipelineContext
    {
        AppContext& app_context;
        BuildContext build_context;
        fs::path prefix_path;
        PipelineState state = PipelineState::Uninitialized;
        BuildCache build_cache;
    };

    // Get expected previous state for next state
    // If next_state is Linked -> expected state is Compiled
    inline constexpr PipelineState get_expected_state(PipelineState next_state)
    {
        if (next_state == PipelineState::Uninitialized)
            return next_state;
        return (PipelineState)(int(next_state) - 1);
    }
    inline bool validate_pipeline_state(PipelineContext& ctx, PipelineState expected_state)
    {
        if (ctx.state != expected_state)
        {
            auto& logger = ctx.app_context.logger;
            logger->log_debug("Incorrect pipeline state. Expected {} got {}", (int)expected_state, (int)ctx.state);
            return false;
        }
        return true;
    }

    void initialize(PipelineContext& ctx, const project::Project& project);
    void advance_build_queue(PipelineContext& ctx);
    void compile(PipelineContext& ctx);
    void link(PipelineContext& ctx);
    void finalize(PipelineContext& ctx);

    void initialize_install_target(PipelineContext& ctx);
    void install(PipelineContext& ctx);
}