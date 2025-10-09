// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/luaapi.h"



export module pragma.client:entities.components.generic;

export import pragma.shared;

export namespace pragma {
	class DLLCLIENT CGenericComponent final : public BaseGenericComponent {
	  public:
		CGenericComponent(BaseEntity &ent) : BaseGenericComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};
