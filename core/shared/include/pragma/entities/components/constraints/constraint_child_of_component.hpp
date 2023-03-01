/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __CONSTRAINT_CHILD_OF_COMPONENT_HPP__
#define __CONSTRAINT_CHILD_OF_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/game/game_coordinate_system.hpp"

namespace pragma {
	class ConstraintComponent;
	class DLLNETWORK ConstraintChildOfComponent final : public BaseEntityComponent {
	  public:
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		ConstraintChildOfComponent(BaseEntity &ent);
		virtual void Initialize() override;

		void SetLocationAxisEnabled(pragma::Axis axis, bool enabled);
		bool IsLocationAxisEnabled(pragma::Axis axis) const;

		void SetRotationAxisEnabled(pragma::Axis axis, bool enabled);
		bool IsRotationAxisEnabled(pragma::Axis axis) const;

		void SetScaleAxisEnabled(pragma::Axis axis, bool enabled);
		bool IsScaleAxisEnabled(pragma::Axis axis) const;

		virtual void InitializeLuaObject(lua_State *lua) override;
	  protected:
		static std::optional<pragma::EntityUComponentMemberRef> FindPosePropertyReference(pragma::BaseEntityComponent &c, pragma::ComponentMemberIndex basePropIdx);
		void ApplyConstraint();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		pragma::ComponentHandle<ConstraintComponent> m_constraintC;
		void UpdateAxisState();
		std::array<bool, 3> m_locationEnabled;
		std::array<bool, 3> m_rotationEnabled;
		std::array<bool, 3> m_scaleEnabled;
		bool m_allAxesEnabled = true;
	};
};

#endif
