// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.server:entities.components.io;

import :entities.components.entity;

export namespace pragma {
	namespace sIOComponent {
		using namespace baseIOComponent;
	}
	class DLLSERVER SIOComponent final : public BaseIOComponent {
	  public:
		SIOComponent(pragma::ecs::BaseEntity &ent) : BaseIOComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};
