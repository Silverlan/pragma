// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/entities/components/base_entity_component.hpp"

export module pragma.shared:entities.components.constraints.copy_rotation;

export namespace pragma {
	class ConstraintComponent;
	class ConstraintSpaceComponent;
	class DLLNETWORK ConstraintCopyRotationComponent final : public BaseEntityComponent {
	  public:
		ConstraintCopyRotationComponent(BaseEntity &ent);
		virtual void Initialize() override;

		virtual void InitializeLuaObject(lua_State *lua) override;
	  protected:
		void ApplyConstraint();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		pragma::ComponentHandle<ConstraintComponent> m_constraintC;
		pragma::ComponentHandle<ConstraintSpaceComponent> m_constraintSpaceC;
	};
};
