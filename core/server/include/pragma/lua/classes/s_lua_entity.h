/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_LUA_ENTITY_H__
#define __S_LUA_ENTITY_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/luaobjectbase.h>
#include <pragma/lua/handle_holder.hpp>

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

#endif
