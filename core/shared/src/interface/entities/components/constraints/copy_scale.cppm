// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.constraints.copy_scale;

export import :entities.components.constraints.space;

export namespace pragma {
	class DLLNETWORK ConstraintCopyScaleComponent final : public BaseEntityComponent {
	  public:
		ConstraintCopyScaleComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;

		virtual void InitializeLuaObject(lua::State *lua) override;
	  protected:
		void ApplyConstraint();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		ComponentHandle<ConstraintComponent> m_constraintC;
		ComponentHandle<ConstraintSpaceComponent> m_constraintSpaceC;
	};
};
