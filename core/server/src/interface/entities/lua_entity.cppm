// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"

#include <string>


export module pragma.server.entities.lua;

import pragma.server.entities.base;

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
};
