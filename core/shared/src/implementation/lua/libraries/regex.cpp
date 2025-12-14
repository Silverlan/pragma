// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

module pragma.shared;

import :scripting.lua.libraries.regex;

DLLNETWORK std::ostream &operator<<(std::ostream &out, const std::match_results<const char *> &match)
{
	out << "RegexResult[" << match.size() << "]";
	return out;
}

int Lua::regex::match(lua::State *l)
{
	auto *s = CheckString(l, 1);
	auto e = CheckString(l, 2);
	std::match_results<const char *> result;
	std::regex_constants::match_flag_type regexFlags {};
	if(IsSet(l, 3))
		regexFlags = static_cast<std::regex_constants::match_flag_type>(CheckInt(l, 3));
	auto r = std::regex_match(s, result, std::regex {e}, regexFlags);
	Lua::PushBool(l, r);
	if(r == true) {
		Lua::Push<std::match_results<const char *>>(l, result);
		return 2;
	}
	return 1;
}

int Lua::regex::search(lua::State *l)
{
	auto *s = CheckString(l, 1);
	auto e = CheckString(l, 2);
	std::match_results<const char *> result;
	int32_t offset = 0;
	if(IsSet(l, 3))
		offset = static_cast<int32_t>(CheckInt(l, 3));
	std::regex_constants::match_flag_type regexFlags {};
	if(IsSet(l, 4))
		regexFlags = static_cast<std::regex_constants::match_flag_type>(CheckInt(l, 4));
	auto r = std::regex_search(s + offset, s + strlen(s), result, std::regex {e}, regexFlags);
	Lua::PushBool(l, r);
	if(r == true) {
		Lua::Push<std::match_results<const char *>>(l, result);
		return 2;
	}
	return 1;
}

std::string Lua::regex::replace(const std::string &input, const std::string &e, const std::string &format, std::regex_constants::match_flag_type regexFlags) { return std::regex_replace(input, std::regex {e}, format, regexFlags); }
std::string Lua::regex::replace(const std::string &input, const std::string &e, const std::string &format) { return replace(input, e, format, {}); }

////////////////////////

void Lua::regex::RegexResult::HasMatch(lua::State *l, std::match_results<const char *> &match) { Lua::PushBool(l, match.empty()); }
void Lua::regex::RegexResult::GetMatchCount(lua::State *l, std::match_results<const char *> &match) { Lua::PushInt(l, match.size()); }
void Lua::regex::RegexResult::GetLength(lua::State *l, std::match_results<const char *> &match) { Lua::PushInt(l, match.length()); }
void Lua::regex::RegexResult::GetPosition(lua::State *l, std::match_results<const char *> &match) { Lua::PushInt(l, match.position()); }
void Lua::regex::RegexResult::GetString(lua::State *l, std::match_results<const char *> &match) { Lua::PushString(l, match.str()); }
void Lua::regex::RegexResult::SetFormat(lua::State *l, std::match_results<const char *> &match, const std::string &format, int32_t flags) { Lua::PushString(l, match.format(format, static_cast<std::regex_constants::match_flag_type>(flags))); }
