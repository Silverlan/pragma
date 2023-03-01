/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
