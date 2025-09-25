// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/game/game_coordinate_system.hpp"

export module pragma.shared:entities.components.constraints.limit_rotation;

export namespace pragma {
	class DLLNETWORK ConstraintLimitRotationComponent final : public BaseEntityComponent {
	  public:
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		ConstraintLimitRotationComponent(BaseEntity &ent);
		virtual void Initialize() override;

		void SetLimit(pragma::Axis axis, const Vector2 &limit);
		const Vector2 &GetLimit(pragma::Axis axis) const;

		void SetLimitEnabled(pragma::Axis axis, bool enabled);
		bool IsLimitEnabled(pragma::Axis axis) const;

		virtual void InitializeLuaObject(lua_State *lua) override;
	  protected:
		void ApplyConstraint();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		pragma::ComponentHandle<ConstraintComponent> m_constraintC;

		std::array<Vector2, umath::to_integral(pragma::Axis::Count)> m_limits {Vector2 {}, Vector2 {}, Vector2 {}};
		std::array<bool, umath::to_integral(pragma::Axis::Count)> m_limitsEnabled {false, false, false};
	};
};
