// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.server:entities.components.lights.point;

import :entities;

export {
	namespace pragma {
		class DLLSERVER SLightPointComponent final : public BaseEnvLightPointComponent {
		  public:
			SLightPointComponent(pragma::ecs::BaseEntity &ent) : BaseEnvLightPointComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		  protected:
			virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		};
	};

	class DLLSERVER EnvLightPoint : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
