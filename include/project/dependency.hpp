#pragma once
#include <string>
#include <filesystem>
#include <vector>
#include <visibility.hpp>

namespace fs = std::filesystem;

namespace rakpak::project
{
    struct Dependency
    {
        std::string name;
        std::string from;
        Visibility visibility = Visibility::Private;
        /* Probably should have version matching here as well */
    };
}