#pragma once
#include <algorithm>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace rakpak::utils
{
    template<typename Item>
    inline bool contains(const std::vector<Item>& container, const Item& item)
    {
        return std::find(container.begin(), container.end(), item) != container.end();
    }

    template<typename Value>
    inline bool contains(const std::set<Value>& container, const Value& item)
    {
        return container.find(item) != container.end();
    }

    template<typename Value>
    inline bool contains(const std::unordered_set<Value>& container, const Value& item)
    {
        return container.find(item) != container.end();
    }

    template<typename Key, typename Value>
    inline bool contains(const std::map<Key, Value>& container, const Value& item)
    {
        return container.find(item) != container.end();
    }

    template<typename Key, typename Value>
    inline bool contains(const std::unordered_map<Key, Value>& container, const Value& item)
    {
        return container.find(item) != container.end();
    }
}