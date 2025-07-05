// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LREGEX_H__
#define __LREGEX_H__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/ldefinitions.h"
#include <regex>

DLLNETWORK std::ostream &operator<<(std::ostream &out, const std::match_results<const char *> &match);

lua_registercheck(RegexResult, std::match_results<const char *>);

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

#endif
