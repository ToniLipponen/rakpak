#include <project/parser.hpp>

using namespace rakpak;
using namespace rakpak::project;

template<>
BuildTarget::Type ProjectParser::get_optional_or<BuildTarget::Type>(
    std::string_view name, 
    const nlohmann::json& json, 
    const BuildTarget::Type& alt)
{
    auto str = get_optional_or<std::string>(name, json);
    if (str == "executable")
        return BuildTarget::Type::Executable;
    else if (str == "library")
        return BuildTarget::Type::Library;
    return alt;
}

template<>
Lang ProjectParser::get_optional_or<Lang>(
    std::string_view name, 
    const nlohmann::json& json, 
    const Lang& alt)
{
    auto str = get_optional_or<std::string>(name, json);
    if (str == "cpp")
        return Lang::Cpp;
    else if (str == "c")
        return Lang::C;
    else if (str == "zig")
        return Lang::Zig;
    return alt;
}

template<>
BuildTarget::Variant ProjectParser::get_optional_or<BuildTarget::Variant>(
    std::string_view name, 
    const nlohmann::json& json, 
    const BuildTarget::Variant& alt)
{
    auto str = get_optional_or<std::string>(name, json);
    if (str == "shared")
        return BuildTarget::Variant::Shared;
    else if (str == "static")
        return BuildTarget::Variant::Static;
    return alt;
}

template<>
Visibility ProjectParser::get_optional_or<Visibility>(
    std::string_view name, 
    const nlohmann::json& json, 
    const Visibility& alt)
{
    auto str = get_optional_or<std::string>(name, json);
    if (str == "public")
        return Visibility::Public;
    else if (str == "private")
        return Visibility::Private;
    return alt;
}

Project ProjectParser::parse_from_directory(const fs::path& path)
{
    m_variables.clear();
    m_target_variables.clear();
    m_project_path = path;
    const auto config_path = path / "rakpak.json";

    logger->log_debug("Parsing {}", config_path.string());
    std::ifstream file_stream(config_path);

    if (!file_stream.is_open())
    {
        logger->log_error("Could not open rakpak.json");
        exit(1);
    }

    Project project;
    nlohmann::json json = nlohmann::json::parse(file_stream, nullptr, true, true);
    parse_metadata(json, project);
    parse_variables(json, m_variables, project);
    parse_profiles(json, project);
    parse_defines(json, project.defines);
    parse_include_paths(json, project.include_paths);
    parse_public_include_paths(json, project.public_include_paths);
    parse_imports(json, project);
    parse_provides(json, project);
    auto targets_json = get_required<nlohmann::json>("targets", json);
    for (const auto& [name, target_json] : targets_json.items())
    {
        BuildTarget target;
        target.build_flags.insert(
            project.build_flags.begin(), 
            project.build_flags.end()
        );
        target.link_flags.insert(
            project.link_flags.begin(), 
            project.link_flags.end()
        );
        target.include_paths.insert(
            project.include_paths.begin(), 
            project.include_paths.end()
        );
        target.include_paths.insert(
            project.public_include_paths.begin(),
            project.public_include_paths.end()
        );
        target.defines.insert(
            target.defines.end(), 
            project.defines.begin(), 
            project.defines.end()
        );
        target.exports.include_paths.insert(
            project.public_include_paths.begin(), 
            project.public_include_paths.end()
        );
        parse_target(target_json, target);
        project.build_targets[target.metadata.name] = std::move(target);
    }
    return project;
}

void ProjectParser::substitute_vars(std::string& str)
{
    for (const auto& [name, val] : m_variables)
        utils::string::alter_replace(str, name, val);
    for (const auto& [name, val] : m_target_variables)
        utils::string::alter_replace(str, name, val);
}

void ProjectParser::parse_metadata(const nlohmann::json& json, BuildTarget& target)
{
    logger->log_debug("Parsing metadata");
    auto& metadata = target.metadata;
    metadata.name = get_required<std::string>("name", json);
    metadata.version = get_optional_or<std::string>("version", json);
    metadata.description = get_optional_or<std::string>("description", json);
    metadata.type = get_optional_or<BuildTarget::Type>("type", json, BuildTarget::Type::Executable);
    metadata.lang = get_optional_or<Lang>("lang", json, Lang::Cpp);
    metadata.variant = get_optional_or<BuildTarget::Variant>("variant", json, BuildTarget::Variant::Static);
    substitute_vars(metadata.name);
    substitute_vars(metadata.description);
    substitute_vars(metadata.output_name);
}

void ProjectParser::parse_metadata(const nlohmann::json& json, Project& project)
{
    logger->log_debug("Parsing metadata");
    auto& metadata = project.metadata;
    metadata.name = get_required<std::string>("name", json);
    metadata.version = get_optional_or<std::string>("version", json);
    metadata.description = get_optional_or<std::string>("description", json);
    metadata.variant = get_optional_or<BuildTarget::Variant>("variant", json, BuildTarget::Variant::Static);
    substitute_vars(metadata.name);
    substitute_vars(metadata.description);
}

void ProjectParser::parse_defines(const nlohmann::json& json, std::vector<Define>& defines)
{
    logger->log_debug("Parsing defines");
    auto defines_json = get_optional<nlohmann::json>("defines", json);
    if (!defines_json.has_value())
        return;
    for (const auto& [define_key, define_props] : defines_json->items())
    {
        project::Define define;
        define.name = define_key;
        define.stringify = get_optional_or<bool>("stringify", define_props);
        define.value = get_optional_or<std::string>("value", define_props);
        defines.push_back(define);
    }
}

void ProjectParser::parse_public_defines(const nlohmann::json& json, std::vector<Define>& defines)
{
    logger->log_debug("Parsing public defines");
    auto defines_json = get_optional<nlohmann::json>("public-defines", json);
    if (!defines_json.has_value())
        return;
    for (const auto& [define_key, define_props] : defines_json->items())
    {
        project::Define define;
        define.name = define_key;
        define.stringify = get_optional_or<bool>("stringify", define_props);
        define.value = get_optional_or<std::string>("value", define_props);
        defines.push_back(define);
    }
}

void ProjectParser::parse_sources(const nlohmann::json& json, BuildTarget& target)
{
    logger->log_debug("Parsing source files");
    auto source_paths = get_required<std::vector<std::string>>("source-files", json);
    for (auto& item : source_paths)
    {
        substitute_vars(item);
        auto expanded = utils::string::expand_pattern(item);
        for (const auto& path_str : expanded)
            target.source_files.insert(fs::path(path_str).lexically_normal());
    }
}

void ProjectParser::parse_include_paths(const nlohmann::json& json, 
    std::unordered_set<fs::path>& include_paths)
{
    logger->log_debug("Parsing include paths");
    auto paths = get_optional<std::vector<std::string>>("include-dirs", json);
    if (!paths.has_value()) return;
    for (auto& path : *paths)
        include_paths.insert(path);
}

void ProjectParser::parse_public_include_paths(const nlohmann::json& json, 
    std::unordered_set<fs::path>& include_paths)
{
    logger->log_debug("Parsing public include paths");
    auto paths = get_optional<std::vector<std::string>>("public-include-dirs", json);
    if (!paths.has_value()) return;
    for (auto& path : *paths)
        include_paths.insert(path);
}

void ProjectParser::parse_dependencies(const nlohmann::json& json, BuildTarget& target)
{
    logger->log_debug("Parsing dependencies");
    const auto&  dependencies_json = get_optional<nlohmann::json>("dependencies", json);
    if (!dependencies_json.has_value())
        return;
    for (const auto& [name, dep_props_json] : dependencies_json->items())
    {
        Dependency dependency;
        dependency.name = name;
        dependency.from = get_required<std::string>("from", dep_props_json);
        dependency.visibility = get_optional_or<Visibility>("visibility", dep_props_json, Visibility::Private);
        target.dependencies.push_back(dependency);
    }
}

void ProjectParser::parse_profiles(const nlohmann::json& json, Project& project)
{
}

void ProjectParser::parse_variables(
    const nlohmann::json& json, 
    std::unordered_map<std::string, std::string>& vars,
    BuildTarget& target)
{
    vars["$(Target.Name)"] = target.metadata.name;
    vars["$(Target.Version)"] = target.metadata.version.string;
    vars["$(Target.Version.Major)"] = target.metadata.version.major;
    vars["$(Target.Version.Minor)"] = target.metadata.version.minor;
    vars["$(Target.Version.Patch)"] = target.metadata.version.patch;
    vars["$(Target.Description)"] = target.metadata.description;
    vars["$(Target.Path)"] = target.metadata.directory;
    parse_variables(json, vars);
}

void ProjectParser::parse_variables(
    const nlohmann::json& json, 
    std::unordered_map<std::string, std::string>& vars,
    Project& project)
{
    auto now = std::chrono::system_clock::now();
    std::time_t utc_time = std::chrono::system_clock::to_time_t(now);
    std::tm utc_tm = *std::gmtime(&utc_time);
    std::tm local_tm = *std::localtime(&utc_time);
    vars["$(Build.Date)"] = fmt::format("{:%Y-%m-%d}", local_tm);
    vars["$(Build.Time)"] = fmt::format("{:%H:%M:%S}", local_tm);
    vars["$(Build.UtcDate)"] = fmt::format("{:%Y-%m-%d}", utc_tm);
    vars["$(Build.UtcTime)"] = fmt::format("{:%H:%M:%S}", utc_tm);

    vars["$(Project.Name)"] = project.metadata.name;
    vars["$(Project.Version)"] = project.metadata.version.string;
    vars["$(Project.Version.Major)"] = project.metadata.version.major;
    vars["$(Project.Version.Minor)"] = project.metadata.version.minor;
    vars["$(Project.Version.Patch)"] = project.metadata.version.patch;
#if defined(__unix__)
    vars["$(Host.OS.Unix)"] = 1;
#endif

#if defined(__linux__)
    vars["$(Host.OS.Linux)"] = 1;
#elif defined(_WIN32)
    vars["$(Host.OS.Windows)"] = 1;
#elif defined(__FreeBSD__)
    vars["$(Host.OS.FreeBSD)"] = 1;
#else
    vars["$(Host.OS.Unkown)"] = 1;
#endif

#if defined(__x86_64__) || defined(_M_X64)
    vars["$(Host.CPU.x86_64)"] = 1;
#elif defined(__i386__) || defined(_M_IX86)
    vars["$(Host.CPU.x86)"] = 1;
#elif defined(__aarch64__) || defined(_M_ARM64)
    vars["$(Host.CPU.arm64)"] = 1;
#elif defined(__arm__) || defined(_M_ARM)
    vars["$(Host.CPU.arm)"] = 1;
#elif defined(__riscv)
    vars["$(Host.CPU.riscv)"] = 1;
#elif defined(__powerpc64__)
    vars["$(Host.CPU.ppc64)"] = 1;
#else
    vars["$(Host.CPU.unknown)"] = 1;
#endif

    parse_variables(json, vars);
}

void ProjectParser::parse_variables(const nlohmann::json& json, 
    std::unordered_map<std::string, std::string>& vars)
{
    auto variables_json = get_optional<nlohmann::json>("variables", json);
    if (!variables_json.has_value())
        return;
    for (const auto& [name, val] : variables_json->items())
    {
        if (val.is_string() && !val.is_null())
        {
            std::string val_str = val;
            substitute_vars(val_str);
            vars["$("+name+")"] = val_str;
        }
        else
            vars["$("+name+")"] = "";
    }
}

void ProjectParser::parse_target(const nlohmann::json& json, BuildTarget& target)
{
    nlohmann::json target_json;
    fs::path path_to_root = m_project_path;

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
        target.metadata.directory = m_project_path / path.remove_filename();
    }
    else
    {
        target_json = json;
        target.metadata.directory = m_project_path;
    }
    parse_metadata(target_json, target);
    parse_variables(target_json, m_target_variables, target);
    parse_include_paths(target_json, target.include_paths);
    parse_public_include_paths(target_json, target.exports.include_paths);
    parse_defines(target_json, target.defines);
    parse_public_defines(target_json, target.exports.defines);
    parse_sources(target_json, target);
    parse_dependencies(target_json, target);
}

void ProjectParser::parse_imports(const nlohmann::json& json, Project& project)
{
    auto subprojects = get_optional<nlohmann::json>("imports", json);
    if (!subprojects.has_value())
        return;
    for (const auto& [name, subproject_json] : subprojects->items())
    {
        fs::path path = get_required<std::string>("path", subproject_json);
        project.subprojects[name] = path.lexically_normal();
    }
}

void ProjectParser::parse_provides(const nlohmann::json& json, Project& project)
{
    auto provides = get_optional<nlohmann::json>("provides", json);
    if (provides.has_value())
        project.provides = *provides;
}