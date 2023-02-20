/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
