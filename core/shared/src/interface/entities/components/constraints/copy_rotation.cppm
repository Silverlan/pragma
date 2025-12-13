// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.constraints.copy_rotation;

export import :entities.components.constraints.base;

export namespace pragma {
	class ConstraintComponent;
	class ConstraintSpaceComponent;
	class DLLNETWORK ConstraintCopyRotationComponent final : public BaseEntityComponent {
	  public:
		ConstraintCopyRotationComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;

		virtual void InitializeLuaObject(lua::State *lua) override;
	  protected:
		void ApplyConstraint();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		ComponentHandle<ConstraintComponent> m_constraintC;
		ComponentHandle<ConstraintSpaceComponent> m_constraintSpaceC;
	};
};
