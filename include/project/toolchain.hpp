#pragma once
#include <string>
#include <optional>
#include <version.hpp>

namespace rakpak::project
{
    struct Toolchain
    {
        enum class Family
        {
            Gcc,
            Clang,
            Msvc,
        };

        std::optional<Family> family;
        std::optional<Version> minimum_version;
    };
}