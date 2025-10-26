// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"

#include <array>
#include <cinttypes>
#include <optional>
#include <vector>
#include <tuple>
#include "pragma/lua/core.hpp"

export module pragma.shared:entities.components.constraints.copy_rotation;

export import :entities.components.constraints.base;

export namespace pragma {
	class ConstraintComponent;
	class ConstraintSpaceComponent;
	class DLLNETWORK ConstraintCopyRotationComponent final : public BaseEntityComponent {
	  public:
		ConstraintCopyRotationComponent(pragma::ecs::BaseEntity &ent);
		virtual void Initialize() override;

		virtual void InitializeLuaObject(lua_State *lua) override;
	  protected:
		void ApplyConstraint();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		pragma::ComponentHandle<ConstraintComponent> m_constraintC;
		pragma::ComponentHandle<ConstraintSpaceComponent> m_constraintSpaceC;
	};
};
