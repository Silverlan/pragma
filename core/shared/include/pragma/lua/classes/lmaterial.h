/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LMATERIAL_H__
#define __LMATERIAL_H__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/ldefinitions.h"

class Material;
namespace Lua
{
	namespace Material
	{
		DLLNETWORK void register_class(luabind::class_<::Material> &classDef);
		DLLNETWORK void IsValid(lua_State *l,::Material &mat);
		DLLNETWORK void GetShaderName(lua_State *l,::Material &mat);
		DLLNETWORK void GetName(lua_State *l,::Material &mat);
		DLLNETWORK void IsTranslucent(lua_State *l,::Material &mat);
		DLLNETWORK void GetDataBlock(lua_State *l,::Material &mat);
	};
};

#endif
