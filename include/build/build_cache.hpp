#pragma once
#include <filesystem>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
namespace fs = std::filesystem;

namespace rakpak
{
    struct BuildCache
    {
        BuildCache();
        ~BuildCache();
        void reset();
        bool needs_rebuild(const fs::path& source_path, const fs::path& object_path);
        // src/file.cpp, obj/debug/src/file.d
        void update_cache(const fs::path& source_path, const fs::path& dep_path);
        void save_to_disk();

    private:
        bool is_hash_cached(const fs::path& path);
        std::size_t get_cached_hash(const fs::path& path);
        std::size_t hash_file_contents(const fs::path& path);
        void load_from_disk();
    private:
        struct DepInfo 
        {
            std::size_t hash; 
            std::size_t timestamp;
        };
        std::mutex m_mutex;
        fs::path m_working_dir;
        std::unordered_map<fs::path, DepInfo> m_dep_cache;
        std::unordered_map<fs::path, std::unordered_set<fs::path>> m_source_cache;
        std::unordered_map<fs::path, std::size_t> m_hash_cache;
    };
}