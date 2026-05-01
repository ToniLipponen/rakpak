#pragma once
#include <common_includes.hpp>
#include <project/build_target.hpp>
#include <package_info.hpp>

namespace rakpak
{
    struct BuildOutput
    {
        struct Binary 
        {
            fs::path root_directory; // ./bin
            fs::path directory; // ./bin/release/
            std::string name; // libExample.so

            fs::path full_path() const 
            {
                return directory / name;
            }
        };

        struct Object
        {
            fs::path root_directory; // ./obj
            fs::path directory; // ./obj/release/
            std::vector<fs::path> objects; // [./obj/release/src/example.o]
        };

        Module package;
        Binary binary;
        Object object;
    };
}