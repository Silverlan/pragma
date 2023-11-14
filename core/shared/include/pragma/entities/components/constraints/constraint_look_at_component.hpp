/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __CONSTRAINT_LOOK_AT_COMPONENT_HPP__
#define __CONSTRAINT_LOOK_AT_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/game/game_coordinate_system.hpp"

namespace pragma {
	class ConstraintComponent;
	class DLLNETWORK ConstraintLookAtComponent final : public BaseEntityComponent {
	  public:
		enum class TrackAxis : uint8_t { X, Y, Z, NegX, NegY, NegZ, Count };
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		ConstraintLookAtComponent(BaseEntity &ent);
		virtual void Initialize() override;

		void SetTrackAxis(TrackAxis axis);
		TrackAxis GetTrackAxis() const;

		virtual void InitializeLuaObject(lua_State *lua) override;
	  protected:
		void ApplyConstraint();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		void ResetDrivenRotation();
		pragma::ComponentHandle<ConstraintComponent> m_constraintC;
		pragma::EntityUComponentMemberRef m_drivenObjectPosition;
		pragma::EntityUComponentMemberRef m_drivenObjectRotation;
		pragma::EntityUComponentMemberRef m_driverPosition;
		bool m_drivenObjectRotationInitialized = false;

		TrackAxis m_trackAxis = TrackAxis::Z;
	};
};

#endif
