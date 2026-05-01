#pragma once
#include <string>
#include <optional>
#include <visibility.hpp>

namespace rakpak::project
{
    struct Define
    {
        std::string name;
        std::optional<std::string> value;
        Visibility visibility = Visibility::Private;
        bool exporting = false;
        bool stringify = false;

        std::string to_string() const
        {
            std::string str = "-D" + name;

            if (value.has_value())
            {
                if (stringify)
                    str = str + "=" + "\"" + value.value() + "\"";
                else
                    str = str + "=" + value.value();
            }

            return str;
        }
    };
}