#pragma once
#include <string_view>
#include <string>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include "enums.hpp"

namespace rakpak::utils::string
{
    inline bool begins_with(std::string_view str, std::string_view prefix)
    {
        if (prefix.size() > str.size()) return false;
        return std::memcmp(str.data(), prefix.data(), prefix.size()) == 0;
    }

    inline bool ends_with(std::string_view str, std::string_view postfix)
    {
        if (postfix.size() > str.size()) return false;
        return std::memcmp(
            str.data() + (str.size() - postfix.size()), 
            postfix.data(), 
            postfix.size()) == 0;
    }

    inline bool contains(std::string_view str, std::string_view sub_str)
    {
        if (sub_str.size() > str.size()) return false;
        return str.find(sub_str) != str.npos;
    }

    inline bool contains(std::string_view str, char c)
    {
        if (str.empty()) return false;
        return str.find(c) != str.npos;
    }

    inline bool consists_of(std::string_view str, std::string_view chars, MatchRule rule = MatchRule::MatchAny)
    {
        if (rule == MatchRule::MatchAny)
        {
            for (auto c : str)
            {
                if (!contains(chars, c))
                    return false;
            }
            return true;
        }
        bool ok = begins_with(str, chars);
        size_t pos = chars.size();
        while (pos < str.size())
        {
            ok = begins_with(str.substr(pos), chars);
            pos += chars.size();
        }
        return ok;
    }
}