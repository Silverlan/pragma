// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.constraints.copy_location;

export import :entities.components.constraints.space;

export namespace pragma {
	class DLLNETWORK ConstraintCopyLocationComponent final : public BaseEntityComponent {
	  public:
		ConstraintCopyLocationComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;

		virtual void InitializeLuaObject(lua::State *lua) override;
	  protected:
		void ApplyConstraint();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		ComponentHandle<ConstraintComponent> m_constraintC;
		ComponentHandle<ConstraintSpaceComponent> m_constraintSpaceC;
	};
};
