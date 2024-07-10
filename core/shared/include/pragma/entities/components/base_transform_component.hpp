/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_TRANSFORM_COMPONENT_HPP__
#define __BASE_TRANSFORM_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/util/coordinate_space.hpp"
#include <sharedutils/property/util_property_vector.h>
#include <sharedutils/property/util_property_quat.hpp>

namespace pragma {
	enum class TransformChangeFlags : uint8_t { None = 0, PositionChanged = 1u, RotationChanged = PositionChanged << 1u, ScaleChanged = RotationChanged << 1u };
	struct DLLNETWORK CEOnPoseChanged : public ComponentEvent {
		CEOnPoseChanged(TransformChangeFlags changeFlags);
		virtual void PushArguments(lua_State *l) override;
		TransformChangeFlags changeFlags;
	};
	class DLLNETWORK BaseTransformComponent : public BaseEntityComponent {
	  public:
		static pragma::ComponentEventId EVENT_ON_POSE_CHANGED;
		static pragma::ComponentEventId EVENT_ON_TELEPORT;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		virtual void Initialize() override;

		void SetPosition(const Vector3 &pos, pragma::CoordinateSpace space);
		void SetPosition(const Vector3 &pos);
		const Vector3 &GetPosition() const;
		Vector3 GetPosition(pragma::CoordinateSpace space) const;
		void SetRotation(const Quat &q, pragma::CoordinateSpace space);
		void SetRotation(const Quat &q);
		const Quat &GetRotation() const;
		Quat GetRotation(pragma::CoordinateSpace space) const;
		void SetPose(const umath::ScaledTransform &pose, pragma::CoordinateSpace space);
		void SetPose(const umath::Transform &pose, pragma::CoordinateSpace space);
		void SetPose(const umath::ScaledTransform &pose);
		void SetPose(const umath::Transform &pose);
		const umath::ScaledTransform &GetPose() const;
		umath::ScaledTransform GetPose(pragma::CoordinateSpace space) const;
		void SetAngles(const EulerAngles &ang, pragma::CoordinateSpace space);
		void SetAngles(const EulerAngles &ang);
		void SetPitch(float pitch, pragma::CoordinateSpace space = pragma::CoordinateSpace::World);
		void SetYaw(float yaw, pragma::CoordinateSpace space = pragma::CoordinateSpace::World);
		void SetRoll(float roll, pragma::CoordinateSpace space = pragma::CoordinateSpace::World);
		EulerAngles GetAngles(pragma::CoordinateSpace space) const;
		EulerAngles GetAngles() const;
		float GetPitch(pragma::CoordinateSpace space = pragma::CoordinateSpace::World) const;
		float GetYaw(pragma::CoordinateSpace space = pragma::CoordinateSpace::World) const;
		float GetRoll(pragma::CoordinateSpace space = pragma::CoordinateSpace::World) const;
		Mat4 GetRotationMatrix(pragma::CoordinateSpace space = pragma::CoordinateSpace::World) const;
		Vector3 GetForward(pragma::CoordinateSpace space = pragma::CoordinateSpace::World) const;
		Vector3 GetUp(pragma::CoordinateSpace space = pragma::CoordinateSpace::World) const;
		Vector3 GetRight(pragma::CoordinateSpace space = pragma::CoordinateSpace::World) const;
		float GetDistance(const Vector3 &p) const;
		float GetDistance(const BaseEntity &ent) const;
		void GetOrientation(Vector3 *forward, Vector3 *right, Vector3 *up = nullptr) const;

		void Teleport(const umath::Transform &targetPose);

		void LocalToWorld(Vector3 *origin) const;
		void LocalToWorld(Quat *rot) const;
		void LocalToWorld(Vector3 *origin, Quat *rot) const;

		void WorldToLocal(Vector3 *origin) const;
		void WorldToLocal(Quat *rot) const;
		void WorldToLocal(Vector3 *origin, Quat *rot) const;

		Vector3 GetOrigin() const;

		Vector3 GetDirection(const BaseEntity &ent, bool bIgnoreYAxis = false) const;
		EulerAngles GetAngles(const BaseEntity &ent, bool bIgnoreYAxis = false) const;
		float GetDotProduct(const BaseEntity &ent, bool bIgnoreYAxis = false) const;
		Vector3 GetDirection(const Vector3 &pos, bool bIgnoreYAxis = false) const;
		EulerAngles GetAngles(const Vector3 &pos, bool bIgnoreYAxis = false) const;
		float GetDotProduct(const Vector3 &pos, bool bIgnoreYAxis = false) const;

		float GetMaxAxisScale() const;
		float GetAbsMaxAxisScale() const;
		const Vector3 &GetScale() const;
		Vector3 GetScale(pragma::CoordinateSpace space) const;
		void SetScale(float scale);
		void SetScale(float scale, pragma::CoordinateSpace space);
		virtual void SetScale(const Vector3 &scale);
		void SetScale(const Vector3 &scale, pragma::CoordinateSpace space);

		virtual Vector3 GetEyePosition() const;
		Vector3 GetEyeOffset() const;
		virtual void SetEyeOffset(const Vector3 &offset);

		double GetLastMoveTime() const;

		void SetPosition(const Vector3 &pos, Bool bForceUpdate, pragma::CoordinateSpace space = pragma::CoordinateSpace::World);

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;

		// Same as SetPosition / SetRotation / SetScale, but don't invoke callbacks
		void SetRawPosition(const Vector3 &pos, pragma::CoordinateSpace space = pragma::CoordinateSpace::World);
		void SetRawRotation(const Quat &rot, pragma::CoordinateSpace space = pragma::CoordinateSpace::World);
		void SetRawScale(const Vector3 &scale, pragma::CoordinateSpace space = pragma::CoordinateSpace::World);

		void UpdateLastMovedTime();
		void OnPoseChanged(TransformChangeFlags changeFlags, bool updatePhysics = true);
	  protected:
		BaseTransformComponent(BaseEntity &ent);
		pragma::NetEventId m_netEvSetScale = pragma::INVALID_NET_EVENT;
		double m_tLastMoved = 0.0; // Last time the entity moved or changed rotation
		Vector3 m_eyeOffset = {};
		umath::ScaledTransform m_pose {};
	};
	struct DLLNETWORK CETeleport : public ComponentEvent {
		CETeleport(const umath::Transform &originalPose, const umath::Transform &targetPose, const umath::Transform &deltaPose);
		virtual void PushArguments(lua_State *l) override;
		umath::Transform originalPose;
		umath::Transform targetPose;
		umath::Transform deltaPose;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::TransformChangeFlags)

#endif
