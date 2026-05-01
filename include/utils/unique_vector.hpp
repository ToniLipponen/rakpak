#pragma once
#include <unordered_set>
#include <vector>
#include <string>

namespace rakpak
{
    template<typename T>
    struct UniqueVector
    {
        void clear()
        {
            m_seen.clear();
            m_vector.clear();
        }

        void push_back(const T& item)
        {
            if (m_seen.insert(item).second)
                m_vector.push_back(item);
        }

        template<typename IteratorType>
        void push_back_range(IteratorType begin, IteratorType end)
        {
            while (begin != end)
            {
                push_back(*begin);
                begin++;
            }
        }

        const std::vector<T>& get_vector() const
        {
            return m_vector;
        }
    private:
        std::unordered_set<T> m_seen;
        std::vector<T> m_vector;
    };

    template<>
    inline void UniqueVector<std::string>::push_back(const std::string& item)
    {
        if (!item.empty())
        {
            if (m_seen.insert(item).second)
                m_vector.push_back(item);
        }
    }
}