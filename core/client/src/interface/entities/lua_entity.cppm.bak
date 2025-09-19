// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/lua/luaobjectbase.h>

export module pragma.client.entities:lua;

export {
	class DLLCLIENT CLuaEntity : public CBaseEntity {
	public:
		CLuaEntity();
		virtual void Initialize() override;
		void SetupLua(const luabind::object &o, const std::string &className);

		void LuaInitialize() {}
		static void default_Initialize(CBaseEntity *ent);
	protected:
		virtual void InitializeLuaObject(lua_State *lua) override;
	};
};

CLuaEntity::CLuaEntity() : CBaseEntity {} {}
void CLuaEntity::Initialize() { CBaseEntity::Initialize(); }
void CLuaEntity::SetupLua(const luabind::object &o, const std::string &className)
{
	m_className = pragma::ents::register_class_name(className);
	SetLuaObject(o);
}
void CLuaEntity::InitializeLuaObject(lua_State *lua) {}

void CLuaEntity::default_Initialize(CBaseEntity *ent) {}
