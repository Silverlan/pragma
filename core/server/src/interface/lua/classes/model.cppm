// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"


export module pragma.server.scripting.lua.classes.model;

export namespace Lua {
	namespace Model {
		namespace Server {
			DLLSERVER void AddMaterial(lua_State *l, ::Model &mdl, uint32_t textureGroup, const std::string &name);
			DLLSERVER void SetMaterial(lua_State *l, ::Model &mdl, uint32_t texIdx, const std::string &name);
		};
	};
};
