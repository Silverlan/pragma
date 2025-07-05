// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/lua/classes/c_lua_entity.h"

CLuaEntity::CLuaEntity() : CBaseEntity {} {}
void CLuaEntity::Initialize() { CBaseEntity::Initialize(); }
void CLuaEntity::SetupLua(const luabind::object &o, const std::string &className)
{
	m_className = pragma::ents::register_class_name(className);
	SetLuaObject(o);
}
void CLuaEntity::InitializeLuaObject(lua_State *lua) {}

void CLuaEntity::default_Initialize(CBaseEntity *ent) {}
