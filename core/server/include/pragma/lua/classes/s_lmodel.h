/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_LMODEL_H__
#define __S_LMODEL_H__

#include "pragma/networkdefinitions.h"
#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>

class Model;
namespace Lua
{
	namespace Model
	{
		namespace Server
		{
			DLLSERVER void AddMaterial(lua_State *l,::Model &mdl,uint32_t textureGroup,const std::string &name);
			DLLSERVER void SetMaterial(lua_State *l,::Model &mdl,uint32_t texIdx,const std::string &name);
		};
	};
};

#endif