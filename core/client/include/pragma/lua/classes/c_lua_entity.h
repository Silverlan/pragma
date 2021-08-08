/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LUA_ENTITY_H__
#define __C_LUA_ENTITY_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/luaobjectbase.h>
#include <pragma/lua/handle_holder.hpp>

class DLLCLIENT CLuaEntity
	: public CBaseEntity
{
public:
	CLuaEntity();
	virtual void Initialize() override;
	void SetupLua(const luabind::object &o,const std::string &className);

	void LuaInitialize() {}
	static void default_Initialize(CBaseEntity *ent);
protected:
	virtual void InitializeLuaObject(lua_State *lua) override;
};

namespace pragma::lua
{
	using CLuaEntityHolder = HandleHolder<CLuaEntity>;
};

#endif
