#pragma once
#include <string>
#include <optional>
#include <version.hpp>

namespace rakpak::project
{
    struct Toolchain
    {
        std::optional<std::string> family;
        std::optional<Version> minimum_version;
    };
}