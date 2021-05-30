/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_LUA_ENTITY_H__
#define __S_LUA_ENTITY_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/luaobjectbase.h>

class DLLSERVER SLuaEntity
	: public SBaseEntity,
	public LuaObjectBase
{
public:
	SLuaEntity(luabind::object &o,const std::string &className);
	virtual void Initialize() override;

	virtual bool IsScripted() const override;
	virtual void DoSpawn() override;
	virtual void Remove() override;
protected:
	virtual void InitializeHandle() override;
	virtual void InitializeLuaObject(lua_State *lua) override;
};
DECLARE_DERIVED_CHILD_HANDLE(DLLSERVER,Entity,BaseEntity,Entity,SLuaEntity,SLuaEntity);

class DLLSERVER SLuaEntityWrapper
	: public SLuaEntityHandle,
	public luabind::wrap_base
{
public:
	void Initialize();
	static void default_Initialize(SLuaEntityWrapper *ent);
};

#endif