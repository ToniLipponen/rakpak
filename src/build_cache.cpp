#include <build_cache.hpp>
#include <utils/string.hpp>
#include <utils/containers.hpp>

#include <fstream>
#include <functional>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>
#include <xxhash.h>

namespace rakpak
{
    BuildCache::BuildCache()
    {
        m_working_dir = fs::current_path();
        load_from_disk();
    }

    BuildCache::~BuildCache()
    {
        save_to_disk();
    }

    void BuildCache::reset()
    {
        m_working_dir = fs::current_path();
        m_dep_cache.clear();
        m_source_cache.clear();
        load_from_disk();
    }

    bool BuildCache::is_hash_cached(const fs::path& path)
    {
        return m_hash_cache.find(path) != m_hash_cache.end();
    }

    std::size_t BuildCache::get_cached_hash(const fs::path& path)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_hash_cache.at(path);
    }

    std::size_t BuildCache::hash_file_contents(const fs::path& path)
    {
        if (is_hash_cached(path))
            return get_cached_hash(path);
        XXH64_state_t* state = XXH64_createState();
        XXH64_reset(state, 0);

        std::ifstream file(path, std::ios::binary);
        char buffer[1024];
        while (true)
        {
            file.read(buffer, sizeof(buffer));
            const std::size_t length = file.gcount();
            if (length == 0) break;
            XXH64_update(state, buffer, length);
        }
        file.close();

        XXH64_hash_t hash = XXH64_digest(state);
        XXH64_freeState(state);
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_hash_cache[path] = hash;
        }
        return hash;
    }

    bool BuildCache::needs_rebuild(const fs::path& source_path, const fs::path& object_path)
    {
        if (!fs::exists(object_path))
            return true;
        auto normal = source_path.lexically_normal();
        if (m_source_cache.find(normal) == m_source_cache.end())
            return true;
        auto dep_list = m_source_cache.at(normal);
        for (const auto& path : dep_list)
        {
            if (m_dep_cache.find(path) == m_dep_cache.end())
                return true;
            std::size_t hash = hash_file_contents(path);
            auto info = m_dep_cache.at(path);
            if (hash != info.hash)
                return true;
        }
        return false;
    }

    void BuildCache::update_cache(const fs::path& source_path, const fs::path& dep_file_path)
    {
        if (!fs::exists(dep_file_path)) return;
        std::ifstream file(dep_file_path, std::ios::binary);
        if (!file) return;
        std::streamsize size = fs::file_size(dep_file_path);

        std::string content;
        content.resize(size * 2);
        if (!file.read(content.data(), size)) return;
        file.close();
        int split_options = utils::string::ExcludeEmpty | utils::string::TrimWhitespace;
        std::vector<std::string_view> object_and_deps = utils::string::split_sv(
            content, 
            ":", 
            utils::string::SplitRule::MatchAny, 
            split_options);
        if (object_and_deps.size() != 2) return;
        std::vector<std::string_view> dep_paths_str_list = utils::string::split_sv(
            object_and_deps.back(),
            " \\\n",
            utils::string::SplitRule::MatchAny,
            split_options
        );
        for (auto path_str : dep_paths_str_list)
        {
            auto trimmed = utils::string::trim_whitespace(std::string_view(path_str));
            if (trimmed.size() == 0)
                continue;
            auto path = fs::path(trimmed).lexically_normal();
            if (fs::exists(path))
            {
                auto source_path_normalized = source_path.lexically_normal();
                DepInfo info;
                info.hash = hash_file_contents(path);
                info.timestamp = fs::last_write_time(path).time_since_epoch().count();
                m_source_cache[source_path_normalized].insert(path);
                m_dep_cache[path] = info;
            }
        }
    }

    void BuildCache::save_to_disk()
    {
        nlohmann::json json;
        json["source-info"] = nlohmann::json::object();
        json["dep-info"] = nlohmann::json::object();
        for (const auto& [path, dep_paths] : m_source_cache)
            json["source-info"][path] = dep_paths;
        for (const auto& [dep_path, dep_info] : m_dep_cache)
        {
            json["dep-info"][dep_path]["hash"] = dep_info.hash;
            json["dep-info"][dep_path]["timestamp"] = dep_info.timestamp;
        }
        auto path = m_working_dir / ".rakpak/cache/";
        if (!fs::exists(path))
            fs::create_directories(path);
        std::ofstream file(path / "build.json");
        file << json.dump(4);
        file.flush();
        file.close();
    }

    void BuildCache::load_from_disk()
    {
        fs::path path = ".rakpak/cache/build.json";
        if (!fs::exists(path)) return;
        std::ifstream file(path);
        nlohmann::json json = nlohmann::json::parse(file);
        const nlohmann::json& source_info_json = json["source-info"];
        for (const auto& [path, dep_list] : source_info_json.items())
            m_source_cache[path] = dep_list;

        const nlohmann::json& dep_info_json = json["dep-info"];
        for (const auto& [path, dep_info] : dep_info_json.items())
        {
            m_dep_cache[path].hash = dep_info["hash"];
            m_dep_cache[path].timestamp = dep_info["timestamp"];
        }
    }
}