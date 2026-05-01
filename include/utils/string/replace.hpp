#pragma once
#include <string>
#include <string_view>
#include <utils/string/contains.hpp>
#include <utils/string/convert.hpp>

namespace rakpak::utils::string
{
    inline std::string replace(std::string original, std::string_view pattern, std::string_view replacement)
    {
        for (size_t pos = original.find(pattern); pos != original.npos; pos = original.find(pattern, pos))
            original.replace(pos, pattern.size(), replacement);
        return original;
    }

    inline bool alter_replace(std::string& original, std::string_view pattern, std::string_view replacement)
    {
        bool modified = false;
        for (size_t pos = original.find(pattern, 0); pos != original.npos; pos = original.find(pattern, pos))
        {
            modified = true;
            original.replace(pos, pattern.size(), replacement);
        }
        return modified;
    }

    inline bool alter_replace(std::string& original, std::string_view pattern, int replacement)
    {
        return alter_replace(original, pattern, std::to_string(replacement));
    }
    inline bool alter_replace(std::string& original, std::string_view pattern, unsigned int replacement)
    {
        return alter_replace(original, pattern, std::to_string(replacement));
    }
    inline bool alter_replace(std::string& original, std::string_view pattern, float replacement)
    {
        return alter_replace(original, pattern, std::to_string(replacement));
    }
    inline bool alter_replace(std::string& original, std::string_view pattern, double replacement)
    {
        return alter_replace(original, pattern, std::to_string(replacement));
    }
}