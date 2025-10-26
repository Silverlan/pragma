// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"

#include "pragma/lua/core.hpp"

export module pragma.shared:entities.components.constraints.copy_scale;

export import :entities.components.constraints.space;

export namespace pragma {
	class DLLNETWORK ConstraintCopyScaleComponent final : public BaseEntityComponent {
	  public:
		ConstraintCopyScaleComponent(pragma::ecs::BaseEntity &ent);
		virtual void Initialize() override;

		virtual void InitializeLuaObject(lua_State *lua) override;
	  protected:
		void ApplyConstraint();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		pragma::ComponentHandle<ConstraintComponent> m_constraintC;
		pragma::ComponentHandle<ConstraintSpaceComponent> m_constraintSpaceC;
	};
};
