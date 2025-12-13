// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.constraints.space;

export import :entities.components.constraints.base;

export namespace pragma {
	class DLLNETWORK ConstraintSpaceComponent final : public BaseEntityComponent {
	  public:
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		ConstraintSpaceComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;

		void SetAxisEnabled(Axis axis, bool enabled);
		bool IsAxisEnabled(Axis axis) const;

		void SetAxisInverted(Axis axis, bool inverted);
		bool IsAxisInverted(Axis axis) const;

		void ApplyFilter(const Vector3 &posDriver, const Vector3 &posDriven, Vector3 &outValue) const;
		void ApplyFilter(const EulerAngles &angDriver, const EulerAngles &angDriven, EulerAngles &outValue) const;

		virtual void InitializeLuaObject(lua::State *lua) override;
	  protected:
		std::array<bool, math::to_integral(Axis::Count)> m_axisEnabled {};
		std::array<bool, math::to_integral(Axis::Count)> m_axisInverted {};
	};
};
