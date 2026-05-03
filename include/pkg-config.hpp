#pragma once
#include <common_includes.hpp>

namespace rakpak::pkg_config
{
    enum class Status
    {
        Ok,
        NotFound,
        Error,
    };

    struct PkgConfigInfo
    {
        Status status = Status::Ok;
        std::string error_message;
        std::vector<std::string> cflags;
        std::vector<std::string> libs;
        std::vector<fs::path> include_paths;
    };

    struct PkgConfigList
    {
        Status status = Status::Ok;
        std::string error_message;
        std::vector<std::string> package_names;
    };

    PkgConfigList get_pkg_list();
    PkgConfigInfo get_pkg(std::string pkg_name);
    PkgConfigInfo get_pkg(std::vector<std::string> pkg_names);
    std::future<PkgConfigInfo> get_pkg_async(std::string pkg_name);
    std::future<PkgConfigInfo> get_pkg_async(std::vector<std::string> pkg_names);
}