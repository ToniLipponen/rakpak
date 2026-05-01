#pragma once
#include <charconv>
#include <string_view>

namespace rakpak::utils::string
{
    template<typename T>
    inline bool try_convert(std::string_view str, T& out)
    {
        if (str.empty()) return false;
        auto begin = str.begin();
        auto end = begin + str.size();
        auto result = std::from_chars(begin, end, out);
        return result.ec == std::errc() && result.ptr == end;
    }
}