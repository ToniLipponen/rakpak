#pragma once
#include <common_includes.hpp>
#include <define.hpp>
#include <dependency.hpp>
#include <module_provider.hpp>
#include <build_target_enums.hpp>

namespace rakpak::project
{
    struct BuildTarget
    {
        using Type = FragmentType;
        using Variant = Linkage;

        struct Exports
        {
            std::vector<Define> defines;
            std::unordered_set<fs::path> include_paths;
            std::vector<std::string> link_flags;
        };
        struct Metadata
        {
            std::string name;
            Version version;
            std::string description;
            std::string output_name;
            fs::path directory;
            Type type = Type::Library;
            Lang lang = Lang::Cpp;
            Variant variant = Variant::Static;
        };

        Metadata metadata;
        Exports exports;

        std::unordered_set<std::string> build_flags;
        std::unordered_set<std::string> link_flags;
        std::unordered_set<fs::path> include_paths;
        std::vector<Define> defines;
        std::vector<Dependency> dependencies;
        std::unordered_set<fs::path> source_files;

        Module to_module() const 
        {
            Module _module;
            _module.linkage = metadata.variant;
            _module.name = metadata.name;
            _module.type = metadata.type;
            if (!metadata.output_name.empty())
                _module.usage_requirements.link_flags.push_back("-l" + metadata.output_name);
            else 
                _module.usage_requirements.link_flags.push_back("-l" + metadata.name);
            
            for (const auto& path : exports.include_paths)
                _module.usage_requirements.include_paths.push_back(fs::absolute(path));
            for (const auto& flag : exports.link_flags)
                _module.usage_requirements.link_flags.push_back(flag);
            for (const auto& define : exports.defines)
                _module.usage_requirements.build_flags.push_back(define.to_string());
            return _module;
        }
    };
}