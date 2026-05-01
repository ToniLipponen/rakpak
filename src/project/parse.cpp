#include <project/project.hpp>
#include <constants.hpp>
#include <utils/string.hpp>
#include <nlohmann/json.hpp>
#include <dependency_source.hpp>
#include <ctime>

/*TODO: Clean this dog shit */
namespace rakpak::project
{
    static std::unordered_map<std::string, std::string> s_project_variables;

    static void substitute_vars(std::string& str)
    {
        for (const auto& [name, val] : s_project_variables)
            utils::string::alter_replace(str, name, val);
    }

    static void substitute_vars(std::string& str, const BuildTarget& target)
    {
        auto& tmeta = target.metadata;
        utils::string::alter_replace(str, "$(Target.Name)", tmeta.name);
        utils::string::alter_replace(str, "$(Target.Version)", tmeta.version.string);
        utils::string::alter_replace(str, "$(Target.Version.Major)", tmeta.version.major);
        utils::string::alter_replace(str, "$(Target.Version.Minor)", tmeta.version.minor);
        utils::string::alter_replace(str, "$(Target.Version.Patch)", tmeta.version.patch);
        utils::string::alter_replace(str, "$(Target.Description)", tmeta.description);
        utils::string::alter_replace(str, "$(Target.Path)", tmeta.directory.c_str());
        substitute_vars(str);
    }

    template <typename T>
    static T get_required(std::string_view name, const nlohmann::json& json, LoggerBase* logger)
    {
        if (!json.contains(name))
        {
            logger->log_error("Missing required property '{}'", name);
            exit(1);
        }
        try
        {
            return (T)json[name];
        }
        catch(const nlohmann::json::exception& e)
        {
            logger->log_error("{}", e.what());
            exit(1);
        }
        return (T){};
    }

    template <typename T>
    static T get_optional(std::string_view name, const nlohmann::json& json, LoggerBase* logger)
    {
        if (!json.contains(name))
            return {};
        try
        {
            return (T)json[name];
        }
        catch(const nlohmann::json::exception& e)
        {
            logger->log_error("{}", e.what());
            exit(1);
        }
        return (T){};
    }

    static void parse_append_str_list(std::string_view name, const nlohmann::json& json, std::vector<std::string>& list, const BuildTarget& target, LoggerBase* logger)
    {
        auto _new = get_optional<std::vector<std::string>>(name, json, logger);
        for (auto& str : _new)
            substitute_vars(str, target);
        list.insert(list.end(), _new.begin(), _new.end());
    }

    static void parse_append_str_list(std::string_view name, const nlohmann::json& json, std::vector<std::string>& list, LoggerBase* logger)
    {
        auto _new = get_optional<std::vector<std::string>>(name, json, logger);
        for (auto& str : _new)
            substitute_vars(str);
        list.insert(list.end(), _new.begin(), _new.end());
    }

    static void parse_metadata(const nlohmann::json& json, project::BuildTarget::Metadata& metadata, LoggerBase* logger)
    {
        logger->log_debug("Parsing target metadata");
        std::string target_name = get_optional<std::string>("name", json, logger);
        if (!target_name.empty())
            metadata.name = target_name;
        auto version = get_optional<std::string>("version", json, logger);
        metadata.description = get_optional<std::string>("description", json, logger);
        metadata.output_name = get_optional<std::string>("output-name", json, logger);
        if (get_optional<std::string>("type", json, logger) == "executable") 
            metadata.type = project::BuildTarget::Type::Executable;
        if (get_optional<std::string>("lang", json, logger) == "c")
            metadata.lang = Lang::C;
        if (get_optional<std::string>("variant", json, logger) == "shared")
            metadata.variant = project::BuildTarget::Variant::Shared;
        logger->log_debug("Successfully parsed target metadata");
        substitute_vars(metadata.name);
        substitute_vars(version);
        metadata.version = version;
        substitute_vars(metadata.description);
    }

    static void parse_include_paths(const nlohmann::json& json, project::BuildTarget& target, LoggerBase* logger)
    {
        std::vector<std::string> include_dirs;
        parse_append_str_list("public-include-dirs", json, include_dirs, logger);
        for (auto& include_path : include_dirs)
        {
            substitute_vars(include_path, target);
            target.exports.include_directories.insert(include_path);
        }
        parse_append_str_list("include-dirs", json, include_dirs, logger);
        for (auto& include_path : include_dirs)
        {
            substitute_vars(include_path, target);
            target.include_directories.insert(include_path);
        }
    }

    static void parse_include_paths(const nlohmann::json& json, Project& project, LoggerBase* logger)
    {
        std::vector<std::string> include_dirs;
        parse_append_str_list("public-include-dirs", json, include_dirs, logger);
        for (auto& include_path : include_dirs)
        {
            substitute_vars(include_path);
            project.public_include_directories.insert(include_path);
        }
        parse_append_str_list("include-dirs", json, include_dirs, logger);
        for (auto& include_path : include_dirs)
        {
            substitute_vars(include_path);
            project.include_directories.insert(include_path);
        }
    }

    static void parse_defines(
        const nlohmann::json& json, 
        std::vector<Define>& defines, 
        std::vector<Define>& exported_defines, 
        LoggerBase* logger)
    {
        logger->log_debug("Parsing defines");
        nlohmann::json defines_json = get_optional<nlohmann::json>("defines", json, logger);
        if (defines_json.empty())
            return;
        if (!defines_json.is_object())
        {
            logger->log_error("Error parsing defines");
            exit(1);
        }
        for (const auto& [define_key, define_props] : defines_json.items())
        {
            project::Define define;
            define.name = define_key;
            if (!define_props.is_object())
            {
                logger->log_warning("Project file format error on defines");
                continue;
            }
            if (!define_props.empty())
            {
                define.stringify = get_optional<bool>("stringify", define_props, logger);
                define.value = get_optional<std::string>("value", define_props, logger);
            }
            defines.push_back(define);
        }
        defines_json = get_optional<nlohmann::json>("public-defines", json, logger);
        if (defines_json.empty())
            return;
        if (!defines_json.is_object())
        {
            logger->log_error("Error parsing defines");
            exit(1);
        }
        for (const auto& [define_key, define_props] : defines_json.items())
        {
            project::Define define;
            define.name = define_key;
            if (!define_props.is_object())
            {
                logger->log_warning("Project file format error on defines");
                continue;
            }
            if (!define_props.empty())
            {
                define.stringify = get_optional<bool>("stringify", define_props, logger);
                define.value = get_optional<std::string>("value", define_props, logger);
            }
            exported_defines.push_back(define);
            defines.push_back(define);
        }
    }

    static void parse_source_files(const nlohmann::json& json, project::BuildTarget& target, LoggerBase* logger)
    {
        logger->log_debug("Parsing source files");
        auto source_paths = get_required<std::vector<std::string>>("source-files", json, logger);
        for (auto& item : source_paths)
        {
            substitute_vars(item, target);
            auto expanded = utils::string::expand_pattern(item);
            for (const auto& path_str : expanded)
                target.source_files.insert(path_str);
        }
    }

    static void parse_dependencies(const nlohmann::json& json, project::BuildTarget& target, LoggerBase* logger)
    {
        logger->log_debug("Parsing dependencies");
        const auto&  dependencies_json = get_optional<nlohmann::json>("dependencies", json, logger);
        if (dependencies_json.empty())
            return;
        if (!dependencies_json.is_object())
        {
            logger->log_warning("Failed to parse dependencies -> needs to be json object");
            exit(1);
        }
        for (const auto& [name, dep_props_json] : dependencies_json.items())
        {
            if (!dep_props_json.is_object())
            {
                logger->log_warning("Project file format error on dependencies");
                exit(1);
            }
            Dependency dependency;
            dependency.name = name;
            dependency.from = get_required<std::string>("from", dep_props_json, logger);
            dependency.visibility = project::Visibility::Private;
            const auto& visibility_str = get_optional<std::string>("visibility", dep_props_json, logger);
            if (visibility_str == "public" || visibility_str == "interface")
                dependency.visibility = project::Visibility::Public;
            target.dependencies.push_back(dependency);
        }
    }

    static void parse_profiles(const nlohmann::json& json, Project& project, LoggerBase* logger)
    {
        auto profiles_json = get_optional<nlohmann::json>("profiles", json, logger);
        if (profiles_json.empty())
            return;
        for (const auto& [profile_name, profile_json] : profiles_json.items())
        {
            BuildProfile profile;
            profile.name = profile_name;
            parse_append_str_list("link-flags", profile_json, profile.link_flags, logger);
            parse_append_str_list("build-flags", profile_json, profile.build_flags, logger);
            parse_defines(profile_json, profile.defines, profile.defines, logger);
            project.build_profiles[profile_name] = profile;
        }
    }

    static void target_replace_str_vars(project::BuildTarget& target)
    {
        auto& tmeta = target.metadata;
        substitute_vars(tmeta.name, target);
        substitute_vars(tmeta.output_name, target);
        substitute_vars(tmeta.version.string, target);
        tmeta.version = tmeta.version.string;
        substitute_vars(tmeta.description, target);

        auto define_replace_str_vars = [&target](Define& def)
        {
            substitute_vars(def.name, target);
            if (def.value.has_value())
            {
                auto& def_val = def.value.value();
                substitute_vars(def_val, target);
            }
        };
        for (auto& def : target.defines)
            define_replace_str_vars(def);
        for (auto& def : target.exports.defines)
            define_replace_str_vars(def);
    }

    static void parse_variables(const nlohmann::json& json, LoggerBase* logger)
    {
        auto variables_json = get_optional<nlohmann::json>("variables", json, logger);
        if (!variables_json.empty())
        {
            for (const auto& [name, val] : variables_json.items())
            {
                if (val.is_string() && !val.is_null())
                {
                    if (utils::string::contains(name, ".{}$%\"'?|"))
                    {
                        logger->log_warning("Illegal symbol in variable name '{}' discarding", name);
                        continue;
                    }
                    std::string val_str = val;
                    substitute_vars(val_str);
                    s_project_variables["$("+name+")"] = val_str;
                }
                else
                    s_project_variables["$("+name+")"] = "";
            }
        }
    }
    static void parse_target(const nlohmann::json& json, project::Project& project, project::BuildTarget& target, LoggerBase* logger)
    {
        nlohmann::json target_json;
        fs::path path_to_root = project.metadata.directory;

        if (json.is_string())
        {
            fs::path path((const std::string&)json);

            if (fs::is_directory(path))
                path.append("rakpak-target.json");
            else if (path.filename() != "rakpak-target.json")
            {
                logger->log_error("Rakpak target files have to be named 'rakpak-target.json'");
                exit(1);
            }
            if (!fs::exists(path))
            {
                logger->log_error("Target path doesn't exist");
                exit(1);
            }
        
            fs::path root_absolute_path = fs::absolute(path_to_root);
            path_to_root = fs::relative(root_absolute_path, path);
            std::ifstream file(path);
            target_json = nlohmann::json::parse(file, nullptr, true, true);
            target.metadata.directory = project.metadata.directory / path.remove_filename();
        }
        else
        {
            target_json = json;
            target.metadata.directory = project.metadata.directory;
        }
        parse_metadata(target_json, target.metadata, logger);
        parse_variables(target_json, logger);
        parse_include_paths(target_json, target, logger);
        parse_defines(target_json, target.defines, target.exports.defines, logger);
        parse_source_files(target_json, target, logger);
        parse_dependencies(target_json, target, logger);
        target_replace_str_vars(target);
    }

    static void project_replace_str_vars(project::Project& project)
    {
    }

    static void parse_variables(const nlohmann::json& json, Project& project, LoggerBase* logger)
    {
        /*
        $(var) string
        $[var] list expansion
        $(var[]) stringify list
        $(var[], ) stringify list with ", " delimiter
        */
        auto& pmeta = project.metadata;
        s_project_variables["$(Project.Name)"] = pmeta.name;
        s_project_variables["$(Project.Version)"] = pmeta.version.string;
        s_project_variables["$(Project.Version.Major)"] = std::to_string(pmeta.version.major);
        s_project_variables["$(Project.Version.Minor)"] = std::to_string(pmeta.version.minor);
        s_project_variables["$(Project.Version.Patch)"] = std::to_string(pmeta.version.patch);
        s_project_variables["$(Project.Description)"] = pmeta.description;

        auto now = std::chrono::system_clock::now();
        
        std::time_t utc_time = std::chrono::system_clock::to_time_t(now);
        std::tm utc_tm = *std::gmtime(&utc_time);
        std::tm local_tm = *std::localtime(&utc_time);

        s_project_variables["$(Build.Date)"] = fmt::format("{:%Y-%m-%d}", local_tm);
        s_project_variables["$(Build.Time)"] = fmt::format("{:%H:%M:%S}", local_tm);
        s_project_variables["$(Build.UtcDate)"] = fmt::format("{:%Y-%m-%d}", utc_tm);
        s_project_variables["$(Build.UtcTime)"] = fmt::format("{:%H:%M:%S}", utc_tm);

        auto variables_json = get_optional<nlohmann::json>("variables", json, logger);
        if (!variables_json.empty())
        {
            for (const auto& [name, val] : variables_json.items())
            {
                if (val.is_string() && !val.is_null())
                {
                    std::string val_str = val;
                    substitute_vars(val_str);
                    s_project_variables["$("+name+")"] = val_str;
                }
                else
                    s_project_variables["$("+name+")"] = "";
            }
        }
    }

    Project parse_from_directory(
        AppContext& app_context,
        const fs::path& project_path) 
    {
        s_project_variables.clear();
        auto logger = app_context.logger.get();
        auto log_scope = logger->scope("Parse");

        const auto config_path = project_path / "rakpak.json";

        logger->log_debug("Parsing {}", config_path.string());
        std::ifstream file_stream(config_path);

        if (!file_stream.is_open())
        {
            logger->log_error("Could not open rakpak.json");
            exit(1);
        }

        nlohmann::json json = nlohmann::json::parse(file_stream, nullptr, true, true);

        Project project;
        project.metadata.directory = project_path;
        project.metadata.name = get_required<std::string>("name", json, logger);
        project.metadata.version = get_optional<std::string>("version", json, logger);
        project.build_flags = get_optional<std::unordered_set<std::string>>("build-flags", json, logger);
        s_project_variables["$(Build.Profile)"] = app_context.build_options.profile;
        parse_variables(json, project, logger);
        auto ignore = project.defines;
        parse_defines(json, project.defines, ignore, logger);
        parse_profiles(json, project, logger);
        parse_include_paths(json, project, logger);
        nlohmann::json subprojects_json = get_optional<nlohmann::json>("imports", json, logger);
        for (const auto& [name, subproject_json] : subprojects_json.items())
        {
            fs::path path = subproject_json["path"];
            project.subprojects[name] = path;
        }
        // Provides
        auto provides_json = get_optional<nlohmann::json>("provides", json, logger);
        if (!provides_json.empty())
            project.provides = provides_json;
        //
        auto variant_str = get_optional<std::string>("variant", json, logger);
        if (variant_str == "shared")
            project.metadata.variant = BuildTarget::Variant::Shared;
        else if (variant_str == "static")
            project.metadata.variant = BuildTarget::Variant::Static;
        //parse_defines(json, project.defines, project.defines, logger);
        nlohmann::json targets_json = get_required<nlohmann::json>("targets", json, logger);
        for (const auto& [name, target_json] : targets_json.items())
        {
            project::BuildTarget target;
            target.metadata.name = name;
            target.build_flags = project.build_flags;
            target.defines = project.defines;
            target.include_directories = project.include_directories;
            target.exports.include_directories = project.public_include_directories;
            parse_target(target_json, project, target, logger);
            if (project.metadata.variant.has_value())
                target.metadata.variant = project.metadata.variant.value();
            project.build_targets[name] = target;
        }
        app_context.logger->log_debug("Finished parsing");

        return project;
    }
}