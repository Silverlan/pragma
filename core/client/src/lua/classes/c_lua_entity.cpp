/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/c_lua_entity.h"

CLuaEntity::CLuaEntity(luabind::object &o,const std::string &className)
	: CBaseEntity(),LuaObjectBase(o)
{
	m_class = className;
	m_luaObj = std::make_unique<luabind::object>(o);
}
void CLuaEntity::Initialize()
{
	CBaseEntity::Initialize();
	CallLuaMember("Initialize");
}
void CLuaEntity::InitializeLuaObject(lua_State*) {}
void CLuaEntity::InitializeHandle()
{
	auto *hEntity = luabind::object_cast_nothrow<util::WeakHandle<BaseEntity>*>(*m_luaObj,static_cast<util::WeakHandle<BaseEntity>*>(nullptr));
	*hEntity = util::WeakHandle<BaseEntity>{std::shared_ptr<BaseEntity>{this,[](BaseEntity*) {}}};
	m_bExternalHandle = true;
}

void CLuaEntity::default_Initialize(CBaseEntity *ent) {}
