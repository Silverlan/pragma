// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.constraints.limit_rotation;

export import :entities.components.constraints.base;
export import :game.coordinate_system;

export namespace pragma {
	class DLLNETWORK ConstraintLimitRotationComponent final : public BaseEntityComponent {
	  public:
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		ConstraintLimitRotationComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;

		void SetLimit(Axis axis, const Vector2 &limit);
		const Vector2 &GetLimit(Axis axis) const;

		void SetLimitEnabled(Axis axis, bool enabled);
		bool IsLimitEnabled(Axis axis) const;

		virtual void InitializeLuaObject(lua::State *lua) override;
	  protected:
		void ApplyConstraint();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		ComponentHandle<ConstraintComponent> m_constraintC;

		std::array<Vector2, math::to_integral(Axis::Count)> m_limits {Vector2 {}, Vector2 {}, Vector2 {}};
		std::array<bool, math::to_integral(Axis::Count)> m_limitsEnabled {false, false, false};
	};
};
