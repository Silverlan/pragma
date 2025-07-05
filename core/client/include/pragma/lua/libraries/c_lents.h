// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_LENTS_H__
#define __C_LENTS_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace Lua {
	namespace ents {
		namespace Client {
			DLLCLIENT int get_local_player(lua_State *l);
			DLLCLIENT int get_listener(lua_State *l);
			DLLCLIENT int get_view_model(lua_State *l);
			DLLCLIENT int get_view_body(lua_State *l);
			DLLCLIENT int get_instance_buffer(lua_State *l);
			DLLCLIENT int get_instance_bone_buffer(lua_State *l);
		};
	};
};

#endif
