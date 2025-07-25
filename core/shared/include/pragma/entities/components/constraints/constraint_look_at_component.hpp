// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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

		void SetUpTarget(const pragma::EntityUComponentMemberRef &upTarget);
		const pragma::EntityUComponentMemberRef &GetUpTarget() const;

		virtual void InitializeLuaObject(lua_State *lua) override;
	  protected:
		void ApplyConstraint();
		static std::optional<pragma::EntityUComponentMemberRef> FindPoseProperty(const pragma::BaseEntityComponent &c, pragma::ComponentMemberIndex basePropIdx);
		std::pair<pragma::BaseEntityComponent *, pragma::ComponentMemberIndex> UpdateUpTarget();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		void ResetDrivenRotation();
		pragma::ComponentHandle<ConstraintComponent> m_constraintC;
		pragma::EntityUComponentMemberRef m_drivenObjectPosition;
		pragma::EntityUComponentMemberRef m_drivenObjectRotation;
		pragma::EntityUComponentMemberRef m_driverPosition;
		bool m_drivenObjectRotationInitialized = false;
		pragma::EntityUComponentMemberRef m_upTarget;

		TrackAxis m_trackAxis = TrackAxis::Z;
	};
};

#endif
