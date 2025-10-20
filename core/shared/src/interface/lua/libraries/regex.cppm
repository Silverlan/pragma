// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <regex>

#include "pragma/lua/core.hpp"

export module pragma.shared:scripting.lua.libraries.regex;

export {
	DLLNETWORK std::ostream &operator<<(std::ostream &out, const std::match_results<const char *> &match);

	namespace Lua {
		namespace regex {
			DLLNETWORK int match(lua_State *l);
			DLLNETWORK int search(lua_State *l);
			DLLNETWORK std::string replace(const std::string &input, const std::string &e, const std::string &format, std::regex_constants::match_flag_type regexFlags);
			DLLNETWORK std::string replace(const std::string &input, const std::string &e, const std::string &format);
			namespace RegexResult {
				DLLNETWORK void HasMatch(lua_State *l, std::match_results<const char *> &match);
				DLLNETWORK void GetMatchCount(lua_State *l, std::match_results<const char *> &match);
				DLLNETWORK void GetLength(lua_State *l, std::match_results<const char *> &match);
				DLLNETWORK void GetPosition(lua_State *l, std::match_results<const char *> &match);
				DLLNETWORK void GetString(lua_State *l, std::match_results<const char *> &match);
				DLLNETWORK void SetFormat(lua_State *l, std::match_results<const char *> &match, const std::string &format, int32_t flags);
			};
		};
	};
};
