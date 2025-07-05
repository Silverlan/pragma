// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LRESOURCE_H__
#define __LRESOURCE_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace Lua {
	namespace resource {
		DLLSERVER bool add_file(const std::string &res, bool stream);
		DLLSERVER bool add_file(const std::string &res);
		DLLSERVER bool add_lua_file(const std::string &f);
		DLLSERVER LuaTableObject get_list(lua_State *l);
	};
};

#endif
