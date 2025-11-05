// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.client:entities.components.io;

export import pragma.shared;

export namespace pragma {
	class DLLCLIENT CIOComponent final : public BaseIOComponent {
	  public:
		CIOComponent(pragma::ecs::BaseEntity &ent) : BaseIOComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};
