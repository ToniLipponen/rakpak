#pragma once

namespace rakpak::project
{
    enum class Visibility
    {
        Private, // Only internal -> dont propagate
        Public, // Used internally + propagate up
        Interface, // 
    };
}