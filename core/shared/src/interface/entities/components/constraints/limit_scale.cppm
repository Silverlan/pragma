// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/game/game_coordinate_system.hpp"

export module pragma.shared:entities.components.constraints.limit_scale;

export namespace pragma {
	class DLLNETWORK ConstraintLimitScaleComponent final : public BaseEntityComponent {
	  public:
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		ConstraintLimitScaleComponent(BaseEntity &ent);
		virtual void Initialize() override;

		void SetMinimum(pragma::Axis axis, float value);
		void SetMaximum(pragma::Axis axis, float value);

		float GetMinimum(pragma::Axis axis) const;
		float GetMaximum(pragma::Axis axis) const;

		void SetMinimumEnabled(pragma::Axis axis, bool enabled);
		bool IsMinimumEnabled(pragma::Axis axis) const;

		void SetMaximumEnabled(pragma::Axis axis, bool enabled);
		bool IsMaximumEnabled(pragma::Axis axis) const;

		virtual void InitializeLuaObject(lua_State *lua) override;
	  protected:
		void ApplyConstraint();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		pragma::ComponentHandle<ConstraintComponent> m_constraintC;
		std::array<float, umath::to_integral(pragma::Axis::Count)> m_minimum {0.f, 0.f, 0.f};
		std::array<float, umath::to_integral(pragma::Axis::Count)> m_maximum {0.f, 0.f, 0.f};
		std::array<bool, umath::to_integral(pragma::Axis::Count)> m_minimumEnabled {false, false, false};
		std::array<bool, umath::to_integral(pragma::Axis::Count)> m_maximumEnabled {false, false, false};
	};
};
