#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <project/build_target_enums.hpp>
#include <version.hpp>
#include <usage_requirements.hpp>

namespace rakpak
{
    /* 
        Module is a importable unit with usage requirements.
        Module can be a linkable library type, or just a header only library.
    */
    struct Module
    {
        std::string name; // sfml-system
        UsageRequirements usage_requirements;
        FragmentType type;
        Linkage linkage;
        Version version;
    };

    /* Package is a module provider */
    struct Package
    {
        bool provides(const std::string& module_name) const
        {
            return m_provides.find(module_name) != m_provides.end();
        }

        void add_module(const Module& module_)
        {
            if (provides(module_.name))
            {
                auto _mod = m_provides.at(module_.name);
                *_mod = module_;
                return;
            }
            //m_provides[module_.name] = new Module;
            m_provides[module_.name] = new Module(module_);
        }

        const Module* get_module(const std::string& module_name) const
        {
            if (!provides(module_name))
                return nullptr;
            return m_provides.at(module_name);
        }
    private:
        //std::vector<std::pair<std::string, Module>> m_provides;
        std::unordered_map<std::string, Module*> m_provides;
    };

    /* Should rename this to something different. PackageSourceProvider maybe? */
    struct PackageManager
    {
        static PackageManager& instance()
        {
            static PackageManager instance;
            return instance;
        }

        void add_module(const std::string& package_name, const Module& module)
        {
            if (!exists(package_name))
                m_packages[package_name] = Package{};
            auto& pkg = m_packages.at(package_name);
            pkg.add_module(module);
        }

        const Module* get_module(const std::string& package_name, const std::string& module_name) const
        {
            if (!exists(package_name))
                return nullptr;
            return m_packages.at(package_name).get_module(module_name);
        }
    private:
        bool exists(const std::string& package_name) const
        {
            return m_packages.find(package_name) != m_packages.end();
        }
    private:
        std::unordered_map<std::string, Package> m_packages;
    };
}