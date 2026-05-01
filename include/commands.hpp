#pragma once
#include <build/build_context.hpp>
#include <app_context.hpp>
#include <optional>
#include <string_view>

// Cli command layer
namespace rakpak::cli
{
    void build(AppContext& app_context);

    void run(AppContext& app_context);

    void clean(AppContext& app_context);

    void reset(AppContext& app_context);

    void install(AppContext& app_context);

    void validate(AppContext& app_context);

    void export_compile_commands(AppContext& app_context);

    void generate_from_template(
        AppContext& app_context, 
        std::string_view name, 
        std::optional<std::string_view> project_name = std::nullopt);

    void describe_options(AppContext& app_context);

    void generate_pkg_config(AppContext& app_context);
}