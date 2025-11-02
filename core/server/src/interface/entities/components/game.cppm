// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"



export module pragma.server:entities.components.game;

import :entities;

export {
	namespace pragma {
		class DLLSERVER SGameComponent final : public BaseGameComponent {
		public:
			SGameComponent(pragma::ecs::BaseEntity &ent) : BaseGameComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER SGameEntity : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
