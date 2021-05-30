/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LRESOURCE_H__
#define __LRESOURCE_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace Lua
{
	namespace resource
	{
		DLLSERVER bool add_file(const std::string &res,bool stream);
		DLLSERVER bool add_file(const std::string &res);
		DLLSERVER bool add_lua_file(const std::string &f);
		DLLSERVER LuaTableObject get_list(lua_State *l);
	};
};

#endif
