#pragma once
#include <common_includes.hpp>
#include <project/project.hpp>
#include <project/dependency.hpp>
#include <project/visibility.hpp>
#include <build_output.hpp>
#include <utils/unique_vector.hpp>

namespace rakpak
{
    // TODO: Figure out a better name for this
    struct TU 
    {
        fs::path source_path;
        fs::path object_path;
        fs::path dep_path;
    };

    struct BuildContext
    {
        fs::path project_root;
        std::string project_name;
        std::deque<project::BuildTarget> build_queue;
        project::BuildProfile build_profile;

        project::BuildTarget target;
        UniqueVector<std::string> compiler_args;
        UniqueVector<std::string> linker_args;
        // TODO: Figure out a better name for this
        std::vector<TU> tu_queue;

        //
        BuildOutput build_output;
        std::unordered_map<std::string, std::vector<std::string>> provides;
        // Intra project target dependencies
        Package module_provider;
    };
}