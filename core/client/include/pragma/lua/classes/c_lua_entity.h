// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_LUA_ENTITY_H__
#define __C_LUA_ENTITY_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/luaobjectbase.h>
#include <pragma/lua/handle_holder.hpp>

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

namespace pragma::lua {
	using CLuaEntityHolder = HandleHolder<CLuaEntity>;
};

#endif
