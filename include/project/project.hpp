#pragma once
#include <common_includes.hpp>
#include <app_context.hpp>
#include <define.hpp>
#include <build_target.hpp>

namespace rakpak::project
{
    struct BuildProfile
    {
        std::string name;
        std::vector<Define> defines;
        std::vector<std::string> build_flags;
        std::vector<std::string> link_flags;
    };

    struct Project
    {
        struct Metadata
        {
            std::string name;
            //std::string version;
            Version version;
            std::string description;
            std::optional<BuildTarget::Variant> variant;
            fs::path directory;
        };
        Metadata metadata;

        std::unordered_set<std::string> build_flags;
        std::unordered_set<std::string> link_flags;
        std::unordered_set<fs::path> include_directories;
        std::unordered_set<fs::path> public_include_directories;
        std::vector<Define> defines;

        std::unordered_map<std::string, BuildProfile> build_profiles;
        std::unordered_map<std::string, BuildTarget> build_targets;
        std::unordered_map<std::string, fs::path> subprojects;
        std::unordered_map<std::string, std::vector<std::string>> provides;
    };

    Project parse_from_directory(AppContext& app_context, const fs::path& project_directory);
}