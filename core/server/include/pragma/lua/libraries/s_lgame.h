/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_LGAME_H__
#define __S_LGAME_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>

using LuaModelObject = luabind::object;
namespace Lua {
	namespace game {
		namespace Server {
			DLLSERVER void set_gravity(const Vector3 &gravity);
			DLLSERVER Vector3 get_gravity();
			DLLSERVER LuaModelObject load_model(lua_State *l, const std::string &name);
			DLLSERVER int create_model(lua_State *l);
			DLLSERVER int load_map(lua_State *l);
			DLLSERVER void change_level(const std::string &mapName, const std::string &landmarkName);
			DLLSERVER void change_level(const std::string &mapName);
		};
	};
};

#endif
