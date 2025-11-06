// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.server:entities.components.generic;

import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SGenericComponent final : public BaseGenericComponent {
	  public:
		SGenericComponent(pragma::ecs::BaseEntity &ent) : BaseGenericComponent(ent) {}
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};
