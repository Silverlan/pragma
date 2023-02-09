/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __CONSTRAINT_SPACE_COMPONENT_HPP__
#define __CONSTRAINT_SPACE_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/game/game_coordinate_system.hpp"

namespace pragma {
	class DLLNETWORK ConstraintSpaceComponent final : public BaseEntityComponent {
	  public:
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		ConstraintSpaceComponent(BaseEntity &ent);
		virtual void Initialize() override;

		void SetAxisEnabled(pragma::Axis axis, bool enabled);
		bool IsAxisEnabled(pragma::Axis axis) const;

		void SetAxisInverted(pragma::Axis axis, bool inverted);
		bool IsAxisInverted(pragma::Axis axis) const;

		void ApplyFilter(const Vector3 &posDriver, const Vector3 &posDriven, Vector3 &outValue) const;
		void ApplyFilter(const EulerAngles &angDriver, const EulerAngles &angDriven, EulerAngles &outValue) const;

		virtual void InitializeLuaObject(lua_State *lua) override;
	  protected:
		std::array<bool, umath::to_integral(pragma::Axis::Count)> m_axisEnabled {};
		std::array<bool, umath::to_integral(pragma::Axis::Count)> m_axisInverted {};
	};
};

#endif
