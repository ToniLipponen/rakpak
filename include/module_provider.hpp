#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <project/build_target_enums.hpp>
#include <version.hpp>
#include <usage_requirements.hpp>
#include <pkg-config.hpp>

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

    struct ModuleProviderBase
    {
        virtual void add_module(const Module& module) {}
        virtual const Module* get_module(const std::string& module_name) = 0;
    };

    struct PkgConfigModuleProvider : ModuleProviderBase
    {
        PkgConfigModuleProvider()
        {
            auto result = pkg_config::get_pkg_list();
            if (result.status == pkg_config::Status::Ok)
                m_package_names = result.package_names;
        }

        virtual ~PkgConfigModuleProvider() = default;

        const Module* get_module(const std::string& module_name) override
        {
            if (std::find(m_package_names.begin(), m_package_names.end(), module_name) == m_package_names.end())
                return nullptr;
            if (m_provided_modules.find(module_name) == m_provided_modules.end())
            {
                auto result = pkg_config::get_pkg(module_name);
                Module _module;
                _module.usage_requirements.include_paths = result.include_paths;
                _module.usage_requirements.build_flags = result.cflags;
                _module.usage_requirements.link_flags = result.libs;
                _module.name = module_name;
                m_provided_modules[module_name] = std::move(_module);
            }
            return &m_provided_modules.at(module_name);
        }
    private:
        std::vector<std::string> m_package_names;
        std::unordered_map<std::string, Module> m_provided_modules;
    };

    struct ModuleProvider : ModuleProviderBase
    {
        void add_module(const Module& _module) override
        {
            m_provided_modules[_module.name] = _module;
        }

        const Module* get_module(const std::string& module_name) override
        {
            auto it = m_provided_modules.find(module_name);
            if (it == m_provided_modules.end())
                return nullptr;
            return &m_provided_modules.at(module_name);
        }
    private:
        std::unordered_map<std::string, Module> m_provided_modules;
    };

    struct SystemModuleProvider : ModuleProviderBase
    {
        SystemModuleProvider()
        {
            Module threads;
            Module socket;
            Module math;
            Module dynamic_loading;
            Module realtime;
#if defined(__linux__)
            threads.usage_requirements.link_flags.push_back("-lpthread");
            math.usage_requirements.link_flags.push_back("m");
            dynamic_loading.usage_requirements.link_flags.push_back("dl");
            realtime.usage_requirements.link_flags.push_back("rt");
#elif defined(__sun)
            realtime.usage_requirements.link_flags.push_back("rt");
#elif defined(_WIN32)
            thread.usage_requirements.link_flags.push_back("ws2_32");
#endif
            m_provided_modules["threads"] = std::move(threads);
            m_provided_modules["socket"] = std::move(socket);
            m_provided_modules["math"] = std::move(math);
            m_provided_modules["dynamic-loading"] = std::move(dynamic_loading);
        }

        void add_module(const Module& _module) override
        {
            m_provided_modules[_module.name] = _module;
        }

        const Module* get_module(const std::string& module_name) override
        {
            auto it = m_provided_modules.find(module_name);
            if (it != m_provided_modules.end())
                return &it->second;
            Module _module;
            _module.usage_requirements.link_flags.push_back(module_name);
            return &(m_provided_modules[module_name] = std::move(_module));
        }
    private:
        std::unordered_map<std::string, Module> m_provided_modules;
    };

    /* Should rename this to something different. PackageSourceProvider maybe? */
    struct ModuleProviderRegistry
    {
        static ModuleProviderRegistry& instance()
        {
            static ModuleProviderRegistry instance;
            return instance;
        }

        void add_module(const std::string& package_name, const Module& _module)
        {
            auto it = m_module_providers.find(package_name);
            if (it == m_module_providers.end())
            {
                auto provider = std::make_unique<ModuleProvider>();
                provider->add_module(_module);
                m_module_providers[package_name] = std::move(provider);
                return;
            }
            it->second->add_module(_module);
        }

        const Module* get_module(const std::string& package_name, const std::string& module_name)
        {
            auto it = m_module_providers.find(package_name);
            if (it == m_module_providers.end())
                return nullptr;
            return it->second->get_module(module_name);
        }
    private:
        ModuleProviderRegistry()
        {
            m_module_providers["pkg-config"] = std::make_unique<PkgConfigModuleProvider>();
            m_module_providers["system"] = std::make_unique<SystemModuleProvider>();
        }
    private:
        std::unordered_map<std::string, std::unique_ptr<ModuleProviderBase>> m_module_providers;
    };
}