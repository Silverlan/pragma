/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_LLOCALE_H__
#define __C_LLOCALE_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace Lua
{
	namespace Locale
	{
		DLLCLIENT int change_language(lua_State *l);
		DLLCLIENT int get_text(lua_State *l);
		DLLCLIENT int load(lua_State *l);
		DLLCLIENT int get_language(lua_State *l);
		DLLCLIENT int get_languages(lua_State *l);
	};
};

#endif