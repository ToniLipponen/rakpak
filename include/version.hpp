#pragma once
#include <string>
#include <utils/string.hpp>

namespace rakpak
{
    struct Version
    {
        std::string string;

        int major = 0;
        int minor = 0;
        int patch = 0;

        Version() = default;

        Version(std::string string)
        : string(std::move(string))
        {
            try_parse_semantic();
        }

        Version& operator=(std::string string)
        {
            this->string = std::move(string);
            try_parse_semantic();
            return *this;
        }

        bool operator>=(const Version& other)
        {
            if (major < other.major) return false;
            if (minor < other.minor) return false;
            if (patch < other.patch) return false;
            // TODO: Version might not use semantic versioning
            return true;
        }
    private:
        void try_parse_semantic()
        {
            auto parts = utils::string::split_sv(this->string, ".");
            if (parts.size() > 0) (void)utils::string::try_convert<int>(parts[0], major);
            if (parts.size() > 1) (void)utils::string::try_convert<int>(parts[1], minor);
            if (parts.size() > 2) (void)utils::string::try_convert<int>(parts[2], patch);
        }
    };
}