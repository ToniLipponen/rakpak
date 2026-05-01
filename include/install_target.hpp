#pragma once
#include <common_includes.hpp>

namespace rakpak
{
    struct InstallTarget
    {
        fs::path root_path;
        fs::path bin_path;
        fs::path lib_path;
        fs::path include_path;
        fs::path asset_path;
    };
}