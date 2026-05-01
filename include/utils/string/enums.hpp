#pragma once
#include <string_view>
#include <string>
#include <vector>
#include <cctype>

namespace rakpak::utils::string
{
	enum SplitOptions
	{
		TrimWhitespace	= 0x1,
		ExcludeEmpty    = 0x2,
	};

	enum class MatchRule 
	{
		MatchAny,
		MatchFull,
	};

	using SplitRule = MatchRule;
	using TrimRule = MatchRule;
}
