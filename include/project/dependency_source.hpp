#pragma once
#include <package_info.hpp>
#include <map>
#include <algorithm>
#include <optional>

namespace rakpak
{
    /*
    struct DependencySource
    {
        std::unordered_set<std::string> provided_names;
        std::map<std::string, PackageInfo> packages;

        bool provides(const std::string& name)
        {
            return std::find(provided_names.begin(), provided_names.end(), name) != provided_names.end();
        }

        void add_provides(const std::string& name)
        {
            provided_names.insert(name);
        }

        void add_package(PackageInfo package)
        {
            if (!provides(package.name))
                provided_names.insert(package.name);
            packages[package.name] = package;
        }

        std::optional<PackageInfo> get_package(const std::string& name) const
        {
            if (packages.find(name) == packages.end())
                return std::nullopt;
            return packages.at(name);
        }
    };

    struct DependencyManager
    {
        static DependencyManager& get_instance()
        {
            static DependencyManager instance;
            return instance;
        }

        bool exists(std::string name)
        {
            return m_sources.find(name) != m_sources.end();
        }

        void add_package(const std::string& source_name, PackageInfo pkg)
        {
            if (!exists(source_name))
                m_sources[source_name] = {};
            m_sources.at(source_name).add_package(pkg);
        }

        std::optional<PackageInfo> get_package(const std::string& source_name, const std::string& pkg_name)
        {
            if (!exists(source_name))
                return std::nullopt;
            return m_sources.at(source_name).get_package(pkg_name);
        }

    private:
        std::map<std::string, DependencySource> m_sources;
    };
    */
}