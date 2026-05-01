#pragma once
#include <common_includes.hpp>
#include <logger.hpp>

namespace rakpak
{
    enum class InstallScope
    {
        Local,
        User,
        System,
    };

    struct BuildOptions
    {
        std::string profile = "debug";
        size_t jobs = 0;
    };

    struct CleanOptions
    {
        bool recursive = false;
        bool purge = false;
    };

    struct InstallOptions
    {
        InstallScope scope;
        bool replace = false;
        bool update_versioned_symlinks = false;
    };

    struct AppContext
    {
        InstallScope install_scope = InstallScope::Local;
        BuildOptions build_options;
        CleanOptions clean_options;

        std::mutex mutex;
        std::vector<std::string_view> runtime_args; // Arguments to be passed into the executed project
        std::unique_ptr<LoggerBase> logger = std::make_unique<ConsoleLogger>(DEBUG_DEFAULT_LOG_LEVEL);
    };
}