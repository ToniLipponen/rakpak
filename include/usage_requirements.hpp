#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <utils/unique_vector.hpp>
namespace fs = std::filesystem;

namespace rakpak
{
    /* These are the requirements that must be satisfied in order to use a module. */
    struct UsageRequirements
    {
        std::vector<fs::path> include_paths;
        std::vector<std::string> link_flags;
        std::vector<std::string> build_flags;

        UsageRequirements& operator=(const UsageRequirements& o)
        {
            include_paths.insert(include_paths.end(), o.include_paths.begin(), o.include_paths.end());
            link_flags.insert(link_flags.end(), o.link_flags.begin(), o.link_flags.end());
            build_flags.insert(build_flags.end(), o.build_flags.begin(), o.build_flags.end());
            return *this;
        }
    };
}