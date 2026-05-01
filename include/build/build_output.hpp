#pragma once
#include <common_includes.hpp>
#include <project/build_target.hpp>

namespace rakpak
{
    struct BuildOutput
    {
        struct Binary 
        {
            fs::path root_directory; // build/bin
            fs::path directory; // build/bin/<profile>/
            std::string name; // libExample.so

            fs::path full_path() const 
            {
                return directory / name;
            }
        };

        struct Object
        {
            fs::path root_directory; // build/obj/
            fs::path directory; // build/obj/<profile>/
            std::vector<fs::path> objects; // [ build/obj/<profile>/src/example.o ]
        };

        Binary binary;
        Object object;
    };
}