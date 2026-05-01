#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <future>

namespace rakpak::process
{
    template<class Range>
    inline int invoke(std::string_view command, const Range& args)
    {
        std::vector<const char*> argv;
        argv.reserve(std::size(args) + 2);
        argv.push_back(command.data());

        for (const auto& arg : args)
        {
            std::string_view sv(arg);
            argv.push_back(sv.data());
        }

        argv.push_back(nullptr);
        return impl::invoke(argv.data());
    }

    template<class Range>
    inline ProcessResult invoke_capture(std::string_view command, const Range& args)
    {
        std::vector<const char*> argv;
        argv.reserve(std::size(args) + 2);
        argv.push_back(command.data());

        for (const auto& arg : args)
        {
            std::string_view sv(arg);
            argv.push_back(sv.data());
        }

        argv.push_back(nullptr);
        return impl::invoke_capture(argv.data());
    }

    template<class Range>
    inline std::future<int> invoke_async(std::string exec, Range args)
    {
        return std::async(
            std::launch::async,
            [command = std::move(exec), params = std::move(args)]{ return invoke(command, params); } 
        );
    }

    template<class Range>
    inline std::future<ProcessResult> invoke_capture_async(std::string exec, Range args)
    {
        return std::async(
            std::launch::async,
            [command = std::move(exec), params = std::move(args)]{ return invoke(command, params); } 
        );
    }
}