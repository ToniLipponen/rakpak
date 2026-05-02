#pragma once
#include <project.hpp>
#include <app_context.hpp>
#include <nlohmann/json.hpp>

namespace rakpak::project
{
    struct ProjectParser
    {
        ProjectParser(AppContext& app_context)
        : app_context(app_context)
        {
            logger = app_context.logger.get();
        }

        Project parse_from_directory(const fs::path& path);
    private:

        template<typename T>
        T get_required(std::string_view name, const nlohmann::json& json)
        {
            return json[name].get<T>();
        }

        template<typename T>
        std::optional<T> get_optional(std::string_view name, const nlohmann::json& json)
        {
            if (json.contains(name))
                return json[name].get<T>();
            return std::nullopt;
        }

        template<typename T>
        T get_optional_or(std::string_view name, const nlohmann::json& json, const T& alt = {})
        {
            auto opt = get_optional<T>(name, json);
            if (opt.has_value())
                return opt.value();
            return alt;
        }

        void substitute_vars(std::string& str);
        void parse_metadata(const nlohmann::json& json, BuildTarget& target);
        void parse_metadata(const nlohmann::json& json, Project& project);

        void parse_defines(const nlohmann::json& json, std::vector<Define>& defines);
        void parse_public_defines(const nlohmann::json& json, std::vector<Define>& defines);

        void parse_sources(const nlohmann::json& json, BuildTarget& target);

        void parse_include_paths(const nlohmann::json& json, std::unordered_set<fs::path>& paths);
        void parse_public_include_paths(const nlohmann::json& json, std::unordered_set<fs::path>& paths);

        void parse_dependencies(const nlohmann::json& json, BuildTarget& target);
        void parse_profiles(const nlohmann::json& json, Project& project);
        void parse_variables(const nlohmann::json& json, 
            std::unordered_map<std::string, std::string>& vars,
            BuildTarget& target);
        void parse_variables(const nlohmann::json& json, 
            std::unordered_map<std::string, std::string>& vars,
            Project& project);
        void parse_variables(const nlohmann::json& json, 
            std::unordered_map<std::string, std::string>& vars);
        void parse_target(const nlohmann::json& json, BuildTarget& target);

        void parse_imports(const nlohmann::json& json, Project& project);
        void parse_provides(const nlohmann::json& json, Project& project);
    private:
        AppContext& app_context;
        LoggerBase* logger;
        fs::path m_project_path;
        std::unordered_map<std::string, std::string> m_target_variables;
        std::unordered_map<std::string, std::string> m_variables;
    };
}