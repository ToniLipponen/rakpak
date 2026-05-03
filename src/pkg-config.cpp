#include <pkg-config.hpp>
#include <process.hpp>
#include <utils/string.hpp>
#include <array>

/*
TODO:
    Add error handling
    Cache pkg-config output to .rakpak/cache/
*/
namespace rakpak::pkg_config
{
    using namespace process;

    PkgConfigList get_pkg_list()
    {
        std::vector<std::string_view> args = { "--list-package-names" };
        ProcessResult result = process::invoke_capture("pkg-config", args);
        PkgConfigList pkg_config_list;
        if (result.exit_code != 0)
        {
            pkg_config_list.status = Status::Error;
            pkg_config_list.error_message = result.output.error;
            return pkg_config_list;
        }
        if (utils::string::contains(result.output.standard, "not found"))
        {
            pkg_config_list.status = Status::NotFound;
            pkg_config_list.error_message = result.output.standard;
            return pkg_config_list;
        }
        pkg_config_list.package_names = utils::string::split(result.output.standard);
        return pkg_config_list;
    }

    PkgConfigInfo get_pkg(std::string pkg_name)
    {
        return get_pkg(std::vector<std::string>{ pkg_name });
    }

    PkgConfigInfo get_pkg(std::vector<std::string> pkg_names)
    {
        std::vector<std::string_view> args = { "--libs", "--cflags" };
        for (const auto& pkg_name : pkg_names)
            args.push_back(pkg_name);
        ProcessResult result = process::invoke_capture("pkg-config", args);

        PkgConfigInfo pkg_config;
        if (result.exit_code != 0)
        {
            pkg_config.status = Status::Error;
            pkg_config.error_message = result.output.error;
            return pkg_config;
        }
        if (utils::string::contains(result.output.standard, "not found"))
        {
            pkg_config.status = Status::NotFound;
            pkg_config.error_message = result.output.standard;
            return pkg_config;
        }
        std::vector<std::string> output_parts = utils::string::split(result.output.standard);
        for (const auto& part : output_parts)
        {
            if (utils::string::begins_with(part, "-l"))
                pkg_config.libs.push_back(part);
            else if (utils::string::begins_with(part, "-I"))
                pkg_config.include_paths.push_back(part);
            else
                pkg_config.cflags.push_back(part);
        }
        return pkg_config;
    }

    std::future<PkgConfigInfo> get_pkg_async(std::string pkg_name)
    {
        return std::async(
            std::launch::async,
            [pkg = std::move(pkg_name)]{ return get_pkg(pkg); } 
        );
    }

    std::future<PkgConfigInfo> get_pkg_async(std::vector<std::string> pkg_names)
    {
        return std::async(
            std::launch::async,
            [pkgs = std::move(pkg_names)]{ return get_pkg(pkgs); } 
        );
    }
}