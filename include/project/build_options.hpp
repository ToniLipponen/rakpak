#pragma once
#include <string>
#include <optional>

namespace rakpak::project
{
    struct BuildOptions
    {
        std::optional<int> cpp_standard;
        std::optional<int> c_standard;
        std::optional<int> opt_level;
        std::optional<std::string> warnings;
        std::optional<bool> warnings_as_errors;
        std::optional<bool> lto;
        std::optional<bool> debug_info;
    };
}