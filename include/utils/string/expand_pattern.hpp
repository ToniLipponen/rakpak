#pragma once
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace rakpak::utils::string
{
    /// @brief Expand filesystem path pattern. 
    /// @example: ./src/**/*.cpp would expand to
    /// ./src/dir/test.cpp
    /// ./src/main.cpp
    /// @param pattern  Path containing wild cards like ./src/**/*.cpp
    /// @return List of expanded file system paths
    inline std::vector<std::string> expand_pattern(const std::string& pattern)
    {
        std::vector<std::string> result;
        if (pattern.find("**") != std::string::npos)
        {
            auto base = pattern.substr(0, pattern.find("**"));
            auto ext = pattern.substr(pattern.find_last_of('.'));
            for (auto& entry : fs::recursive_directory_iterator(base))
                if (entry.is_regular_file() && entry.path().extension() == ext)
                    result.push_back(entry.path().string());
        }
        else if (pattern.find('*') != std::string::npos)
        {
            auto dir = fs::path(pattern).parent_path();
            auto ext = fs::path(pattern).extension();
            for (auto& entry : fs::directory_iterator(dir))
                if (entry.is_regular_file() && entry.path().extension() == ext)
                    result.push_back(entry.path().string());
        }
        else
            result.push_back(pattern);
        return result;
    }
}