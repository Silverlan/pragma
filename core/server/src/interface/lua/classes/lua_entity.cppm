// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include <pragma/lua/luaobjectbase.h>
#include <pragma/lua/handle_holder.hpp>

export module pragma.server.scripting.lua.classes.lua_entity;

export {
	class DLLSERVER SLuaEntity : public SBaseEntity {
	public:
		SLuaEntity();
		virtual void Initialize() override;

		virtual bool IsScripted() const override;
		virtual void DoSpawn() override;
		virtual void Remove() override;
		void SetupLua(const luabind::object &o, const std::string &className);

		void LuaInitialize() {}
		static void default_Initialize(SBaseEntity *ent);
	protected:
		virtual void InitializeLuaObject(lua_State *lua) override;
	};

	namespace pragma::lua {
		using SLuaEntityHolder = HandleHolder<SLuaEntity>;
	};
};
