#pragma once
#include <functional>
#include "enums.hpp"
#include "trim.hpp"

namespace rakpak::utils::string
{
	inline std::vector<std::string_view> split_sv(
		std::string_view str, 
		std::string_view delim = " \t\n\r", 
		SplitRule rule = SplitRule::MatchAny,
		int options = ExcludeEmpty | TrimWhitespace)
	{
		std::function<size_t(size_t)> find_next = [&](size_t pos)
		{
			return str.find(delim, pos);
		};
		auto find_next_match_any = [&](size_t pos)
		{
			size_t end = str.size();
			for (char delimiter : delim)
			{
				size_t p = str.find(delimiter, pos);
				if (p < end) end = p;
			}
			return end;
		};

		if (rule == SplitRule::MatchAny)
			find_next = find_next_match_any;

		std::vector<std::string_view> parts;
		parts.reserve(128);
		const size_t delim_size = rule == SplitRule::MatchAny ? 1 : delim.size();
		size_t begin = 0;
		size_t pos = 0;
		std::string_view part;

		while (begin < str.size())
		{
			pos = find_next(pos);
			if (pos == std::string::npos)
				break;
			size_t len = pos - begin;
			part = str.substr(begin, len);
			if (options & TrimWhitespace)
			{
				part = trim_whitespace(part);
				len = part.length();
			}
			if (len > 0 || (options & ExcludeEmpty))
				parts.push_back(part);
			begin = pos + delim_size;
			pos = begin;
		}
		if (begin < str.size())
		{
			part = str.substr(begin);
			if (options & TrimWhitespace)
				part = trim_whitespace(part);
			if ((options & ExcludeEmpty) == 0 || !part.empty())
				parts.push_back(part);
		}
		return parts;
	}

	inline std::vector<std::string> split(
		std::string_view str, 
		std::string_view delim = " \t\n\r", 
		SplitRule rule = SplitRule::MatchAny,
		int options = ExcludeEmpty | TrimWhitespace)
	{
		std::function<size_t(size_t)> find_next = [&](size_t pos)
		{
			return str.find(delim, pos);
		};
		auto find_next_match_any = [&](size_t pos)
		{
			size_t end = str.size();
			for (char delimiter : delim)
			{
				size_t p = str.find(delimiter, pos);
				if (p < end) end = p;
			}
			return end;
		};

		if (rule == SplitRule::MatchAny)
			find_next = find_next_match_any;

		std::vector<std::string> parts;
		parts.reserve(128);
		const size_t delim_size = rule == SplitRule::MatchAny ? 1 : delim.size();
		size_t begin = 0;
		size_t pos = 0;
		std::string_view part;

		while (begin < str.size())
		{
			pos = find_next(pos);
			if (pos == std::string::npos)
				break;
			size_t len = pos - begin;
			part = str.substr(begin, len);
			if (options & TrimWhitespace)
			{
				part = trim_whitespace(part);
				len = part.length();
			}
			if (len > 0 || (options & ExcludeEmpty))
				parts.push_back(std::string(part));
			begin = pos + delim_size;
			pos = begin;
		}
		if (begin < str.size())
		{
			part = str.substr(begin);
			if (options & TrimWhitespace)
				part = trim_whitespace(part);
			if ((options & ExcludeEmpty) == 0 || !part.empty())
				parts.push_back(std::string(part));
		}
		return parts;
	}
}
