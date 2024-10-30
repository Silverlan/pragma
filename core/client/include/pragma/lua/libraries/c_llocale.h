/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LLOCALE_H__
#define __C_LLOCALE_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace Lua {
	namespace Locale {
		DLLCLIENT void set_localization(const std::string &id, const std::string &text);
		DLLCLIENT void change_language(const std::string &lan);
		DLLCLIENT int get_text(lua_State *l);
		DLLCLIENT bool load(const std::string &fileName);
		DLLCLIENT const std::string &get_language();
		DLLCLIENT int get_languages(lua_State *l);
		DLLCLIENT bool localize(const std::string &identifier, const std::string &lan, const std::string &category, const std::string &text);
		DLLCLIENT bool relocalize(const std::string &identifier, const std::string &newIdentifier, const std::string &oldCategory, const std::string &newCategory);
		DLLCLIENT void clear();
		DLLCLIENT Lua::map<std::string, std::string> get_texts(lua_State *l);
		DLLCLIENT Lua::opt<Lua::map<std::string, std::string>> parse(lua_State *l, const std::string &fileName, const std::string &lan);
		DLLCLIENT Lua::opt<Lua::map<std::string, std::string>> parse(lua_State *l, const std::string &fileName);
	};
};

#endif
