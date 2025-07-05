// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_LMODEL_H__
#define __S_LMODEL_H__

#include "pragma/networkdefinitions.h"
#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>

class Model;
namespace Lua {
	namespace Model {
		namespace Server {
			DLLSERVER void AddMaterial(lua_State *l, ::Model &mdl, uint32_t textureGroup, const std::string &name);
			DLLSERVER void SetMaterial(lua_State *l, ::Model &mdl, uint32_t texIdx, const std::string &name);
		};
	};
};

#endif
