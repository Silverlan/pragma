// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.constraints.limit_location;

export import :entities.components.constraints.base;

export namespace pragma {
	class DLLNETWORK ConstraintLimitLocationComponent final : public BaseEntityComponent {
	  public:
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		ConstraintLimitLocationComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;

		void SetMinimum(Axis axis, float value);
		void SetMaximum(Axis axis, float value);

		float GetMinimum(Axis axis) const;
		float GetMaximum(Axis axis) const;

		void SetMinimumEnabled(Axis axis, bool enabled);
		bool IsMinimumEnabled(Axis axis) const;

		void SetMaximumEnabled(Axis axis, bool enabled);
		bool IsMaximumEnabled(Axis axis) const;

		virtual void InitializeLuaObject(lua::State *lua) override;
	  protected:
		void ApplyConstraint();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		ComponentHandle<ConstraintComponent> m_constraintC;
		std::array<float, math::to_integral(Axis::Count)> m_minimum {0.f, 0.f, 0.f};
		std::array<float, math::to_integral(Axis::Count)> m_maximum {0.f, 0.f, 0.f};
		std::array<bool, math::to_integral(Axis::Count)> m_minimumEnabled {false, false, false};
		std::array<bool, math::to_integral(Axis::Count)> m_maximumEnabled {false, false, false};
	};
};
