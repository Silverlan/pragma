// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LNET_HPP__
#define __LNET_HPP__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace Lua {
	namespace net {
		DLLNETWORK void RegisterLibraryEnums(lua_State *l);
		DLLNETWORK pragma::NetEventId register_event(lua_State *l, const std::string &name);
	};
};

#endif
