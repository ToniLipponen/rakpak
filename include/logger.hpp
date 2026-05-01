#pragma once
#include <fmt/format.h>
#include <fmt/printf.h>
#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/ostream.h>
#include <fmt/chrono.h>
#include <common_includes.hpp>

#define DEBUG_DEFAULT_LOG_LEVEL LogLevel::Warning 

namespace rakpak
{
    enum LogLevel : int 
    {
        None            = (0 << 0),
        Critical        = (1 << 0),
        Error           = (1 << 1),
        Warning         = (1 << 2),
        Informational   = (1 << 3),
        Success         = (1 << 4), // This probably shouldn't be here
        All             = (1 << 5),
        Extra           = (1 << 6),
        Debug           = (1 << 7), // Only used for internal debugging
    };

    struct LoggerScope
    {
        LoggerScope(std::vector<std::string>& scope_names)
        : m_scope_names(scope_names)
        {
        }

        LoggerScope(const LoggerScope& other)
        : m_scope_names(other.m_scope_names)
        {
            m_scope_names.push_back(m_scope_names.back());
        }

        ~LoggerScope()
        {
            m_scope_names.pop_back();
        }
    private:
        std::vector<std::string>& m_scope_names;
    };

    struct LoggerBase
    {
        virtual ~LoggerBase(){}
        virtual void log(const std::string& message) = 0;

        template<typename ... T>
        void print(const std::string& message, const T& ... args)
        {
            std::string formatted = fmt::format(message, args...);
            std::lock_guard<std::mutex> lock(m_mutex);
            log(formatted);
        }

        template<typename ... T>
        void log_debug(const std::string& message, const T& ... args)
        {
            if (!should_log(LogLevel::Debug)) return;
            std::string formatted = fmt::format(message, args...);
            formatted = fmt::format("{} {}", get_scope_str(LogLevel::Debug), formatted);
            print(formatted);
        }

        template<typename ... T>
        void log_info(const std::string& message, const T& ... args)
        {
            if (!should_log(LogLevel::Informational)) return;
            std::string formatted = fmt::format(message, args...);
            formatted = fmt::format("{} {}", get_scope_str(LogLevel::Informational), formatted);
            print(formatted);
        }

        template<typename ... T>
        void log_warning(const std::string& message, const T& ... args)
        {
            if (!should_log(LogLevel::Warning)) return;
            std::string formatted = fmt::format(message, args...);
            formatted = fmt::format("{} {}", get_scope_str(LogLevel::Warning), formatted);
            print(formatted);
        }

        template<typename ... T>
        void log_error(const std::string& message, const T& ... args)
        {
            if (!should_log(LogLevel::Error))
                return;
            std::string formatted = fmt::format(message, args...);
            formatted = fmt::format("{} {}", get_scope_str(LogLevel::Error), formatted);
            print(formatted);
        }

        template<typename ... T>
        void log_success(const std::string& message, const T& ... args)
        {
            if (!should_log(LogLevel::Informational))
                return;
            std::string formatted = fmt::format(message, args...);
            formatted = fmt::format("{} {}", get_scope_str(LogLevel::Success), formatted);
            print(formatted);
        }

        LoggerScope scope(std::string scope_name = "")
        {
            scope_names.push_back(scope_name);
            return LoggerScope(scope_names);
        }

        bool masked = false;
        int log_level;
        int scope_indent_size = 2;
        inline static thread_local std::vector<std::string> scope_names;
    private:
        std::mutex m_mutex;
        bool should_log(LogLevel level)
        {
            if (masked)
                return log_level & static_cast<int>(level) != 0;
            return log_level >= static_cast<int>(level);
        }
        std::string get_scope_str(LogLevel level)
        {
            std::string severity;
            switch (level)
            {
                case LogLevel::Critical:
                    severity = fmt::format(fmt::fg(fmt::color::red), "{}", "Critical");
                    break;
                case LogLevel::Error:
                    severity = fmt::format(fmt::fg(fmt::color::red), "{}", "Error");
                    break;
                case LogLevel::Warning:
                    severity = fmt::format(fmt::fg(fmt::color::yellow), "{}", "Warning");
                    break;
                case LogLevel::Informational:
                    severity = "Info";
                    break;
                case LogLevel::Extra:
                    severity = fmt::format(fmt::fg(fmt::color::light_blue), "{}", "Extra");
                    break;
                case LogLevel::Success:
                    severity = fmt::format(fmt::fg(fmt::color::green), "{}", "Success");
                    break;
                case LogLevel::Debug:
                    severity = fmt::format(fmt::fg(fmt::color::magenta), "{}", "Debug");
                    break;
                default:
                    break;
            }
            std::string scope_str;
            ssize_t scope_level = static_cast<ssize_t>(scope_names.size()) - 1;
            scope_level = scope_level < 0 ? 0 : scope_level * scope_indent_size;
            if (!scope_names.empty())
                scope_str = fmt::format("{:>{}}[{}][{}]", "", scope_level, severity, scope_names.back());
            else
                scope_str = fmt::format("{:>{}}[{}]", "", scope_level, severity);
            return scope_str;
        }
    };

    struct ConsoleLogger : public LoggerBase
    {
        ConsoleLogger(LogLevel log_level)
        {
            this->log_level = log_level;
        }

        virtual ~ConsoleLogger(){}

        void log(const std::string& message) override
        {
            fmt::println(message);
        }
    };

    struct FileLogger : public LoggerBase
    {
        FileLogger(LogLevel log_level, const std::string& path)
        {
            this->log_level = log_level;
            m_file_stream.open(path);
        }

        virtual ~FileLogger()
        {
            m_file_stream.flush();
            m_file_stream.close();
        }

        void log(const std::string& message) override
        {
            fmt::println(m_file_stream, message);
        }
    private:
        std::ofstream m_file_stream;
    };
}