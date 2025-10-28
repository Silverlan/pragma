// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"



export module pragma.server.entities.components.flashlight;

import pragma.server.entities;

export {
	namespace pragma {
		class DLLSERVER SFlashlightComponent final : public BaseFlashlightComponent {
		public:
			SFlashlightComponent(pragma::ecs::BaseEntity &ent) : BaseFlashlightComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER Flashlight : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
