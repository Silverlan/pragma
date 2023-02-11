/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __CONSTRAINT_TRACK_TO_COMPONENT_HPP__
#define __CONSTRAINT_TRACK_TO_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/game/game_coordinate_system.hpp"

namespace pragma {
	class ConstraintComponent;
	class DLLNETWORK ConstraintTrackToComponent final : public BaseEntityComponent {
	  public:
		enum class TrackAxis : uint8_t { X, Y, Z, NegX, NegY, NegZ };
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		ConstraintTrackToComponent(BaseEntity &ent);
		virtual void Initialize() override;

		void SetTrackAxis(TrackAxis axis);
		TrackAxis GetTrackAxis() const;

		void SetUpAxis(pragma::Axis upAxis);
		pragma::Axis GetUpAxis() const;

		void SetTargetY(bool targetY);
		bool ShouldTargetY() const;

		virtual void InitializeLuaObject(lua_State *lua) override;
	  protected:
		void ApplyConstraint();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		void ResetDrivenRotation();
		pragma::ComponentHandle<ConstraintComponent> m_constraintC;
		pragma::EntityUComponentMemberRef m_drivenObjectRotation;
		bool m_drivenObjectRotationInitialized = false;

		TrackAxis m_trackAxis = TrackAxis::X;
		pragma::Axis m_upAxis = pragma::Axis::Y;
		bool m_targetY = false;
	};
};

#endif
