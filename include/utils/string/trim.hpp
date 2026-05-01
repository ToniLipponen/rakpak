#pragma once
#include "enums.hpp"
#include "contains.hpp"

namespace rakpak::utils::string
{
	inline std::string_view trim(
		std::string_view string, 
		std::string_view trim, 
		TrimRule rule = TrimRule::MatchAny)
	{
		if (string.empty() || trim.empty()) return string;
		const char* const str_beg = string.begin();
		const char* beg = str_beg;
		const char* end = beg + string.size() - 1;
		const size_t trim_size = rule == TrimRule::MatchAny ? 1 : trim.size();

		if (rule == TrimRule::MatchAny)
		{
			while (beg < end && utils::string::contains(trim, *beg)) ++beg;
			while (beg < end && utils::string::contains(trim, *end)) --end;
		}
		else
		{
			while (beg < end && utils::string::begins_with(std::string_view(beg, end - beg), trim)) beg += trim_size;
			while (beg < end && utils::string::ends_with(std::string_view(beg, end - beg), trim)) end -= trim_size;
		}
		return std::string_view(beg, end - beg + 1);
	}

	inline std::string_view trim(std::string_view string, char c)
	{
		if (string.empty())
			return string;
		std::string_view::iterator beg = string.begin();
		std::string_view::iterator end = beg + string.length() - 1;

		while (beg < end)
		{
			const bool trim_beg = *beg == c;
			const bool trim_end = *end == c;
			const bool trim = trim_beg || trim_end;
			if (!trim) break;
			if (trim_beg) ++beg;
			if (trim_end) --end;
		}

		const size_t p = std::distance(string.begin(), beg);
		const size_t n = std::distance(beg, end) + 1;
		return string.substr(p, n);
	}

	inline std::string_view trim_whitespace(std::string_view string)
	{
		const std::string_view ws = " \t\r\n";
		return utils::string::trim(string, ws);
	}

	inline std::string trim_whitespace(const std::string& string)
	{
		if (string.empty())
			return string;
		std::string_view trimmed = trim_whitespace(std::string_view(string));
		return std::string(trimmed);
	}
}
