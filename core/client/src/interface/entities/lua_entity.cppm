// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.lua;

export import :entities.base_entity;

export {
	class DLLCLIENT CLuaEntity : public pragma::ecs::CBaseEntity {
	  public:
		CLuaEntity();
		virtual void Initialize() override;
		void SetupLua(const luabind::object &o, const std::string &className);

		void LuaInitialize() {}
		static void default_Initialize(CBaseEntity *ent);
	  protected:
		virtual void InitializeLuaObject(lua::State *lua) override;
	};
};

CLuaEntity::CLuaEntity() : CBaseEntity {} {}
void CLuaEntity::Initialize() { CBaseEntity::Initialize(); }
void CLuaEntity::SetupLua(const luabind::object &o, const std::string &className)
{
	m_className = pragma::ents::register_class_name(className);
	SetLuaObject(o);
}
void CLuaEntity::InitializeLuaObject(lua::State *lua) {}

void CLuaEntity::default_Initialize(CBaseEntity *ent) {}
