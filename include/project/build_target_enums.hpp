#pragma once

namespace rakpak
{
    enum class Lang 
    {
        C,
        Cpp,
        Zig,
    };

    enum class FragmentType
    {
        Executable,
        Library,
    };

    enum class Linkage 
    {
        Static,
        Shared,
    };
}