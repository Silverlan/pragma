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

class DLLCLIENT CLuaEntity
	: public CBaseEntity,
	public LuaObjectBase,
	public luabind::wrap_base
{
public:
	CLuaEntity(luabind::object &o,const std::string &className);
	virtual void Initialize() override;

	void LuaInitialize() {}
	static void default_Initialize(CBaseEntity *ent);
protected:
	virtual void InitializeHandle() override;
	virtual void InitializeLuaObject(lua_State *lua) override;
};

#endif
