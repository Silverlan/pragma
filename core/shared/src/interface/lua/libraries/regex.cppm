// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.regex;

export import pragma.lua;

export {
	DLLNETWORK std::ostream &operator<<(std::ostream &out, const std::match_results<const char *> &match);

	namespace Lua {
		namespace regex {
			DLLNETWORK int match(lua::State *l);
			DLLNETWORK int search(lua::State *l);
			DLLNETWORK std::string replace(const std::string &input, const std::string &e, const std::string &format, std::regex_constants::match_flag_type regexFlags);
			DLLNETWORK std::string replace(const std::string &input, const std::string &e, const std::string &format);
			namespace RegexResult {
				DLLNETWORK void HasMatch(lua::State *l, std::match_results<const char *> &match);
				DLLNETWORK void GetMatchCount(lua::State *l, std::match_results<const char *> &match);
				DLLNETWORK void GetLength(lua::State *l, std::match_results<const char *> &match);
				DLLNETWORK void GetPosition(lua::State *l, std::match_results<const char *> &match);
				DLLNETWORK void GetString(lua::State *l, std::match_results<const char *> &match);
				DLLNETWORK void SetFormat(lua::State *l, std::match_results<const char *> &match, const std::string &format, int32_t flags);
			};
		};
	};
};
