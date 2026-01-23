// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.constraints.look_at;

export import :entities.components.constraints.base;

export namespace pragma {
	class DLLNETWORK ConstraintLookAtComponent final : public BaseEntityComponent {
	  public:
		enum class TrackAxis : uint8_t { X, Y, Z, NegX, NegY, NegZ, Count };
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		ConstraintLookAtComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;

		void SetTrackAxis(TrackAxis axis);
		TrackAxis GetTrackAxis() const;

		void SetUpTarget(const EntityUComponentMemberRef &upTarget);
		const EntityUComponentMemberRef &GetUpTarget() const;

		virtual void InitializeLuaObject(lua::State *lua) override;
	  protected:
		void ApplyConstraint();
		static std::optional<EntityUComponentMemberRef> FindPoseProperty(const BaseEntityComponent &c, ComponentMemberIndex basePropIdx);
		std::pair<BaseEntityComponent *, ComponentMemberIndex> UpdateUpTarget();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		void ResetDrivenRotation();
		ComponentHandle<ConstraintComponent> m_constraintC;
		EntityUComponentMemberRef m_drivenObjectPosition;
		EntityUComponentMemberRef m_drivenObjectRotation;
		EntityUComponentMemberRef m_driverPosition;
		bool m_drivenObjectRotationInitialized = false;
		EntityUComponentMemberRef m_upTarget;

		TrackAxis m_trackAxis = TrackAxis::Z;
	};
};
