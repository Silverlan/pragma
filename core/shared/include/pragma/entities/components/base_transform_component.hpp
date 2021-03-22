/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_TRANSFORM_COMPONENT_HPP__
#define __BASE_TRANSFORM_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property_vector.h>
#include <sharedutils/property/util_property_quat.hpp>

namespace pragma
{
	enum class TransformChangeFlags : uint8_t
	{
		None = 0,
		PositionChanged = 1u,
		RotationChanged = PositionChanged<<1u,
		ScaleChanged = RotationChanged<<1u
	};
	struct DLLNETWORK CEOnPoseChanged
		: public ComponentEvent
	{
		CEOnPoseChanged(TransformChangeFlags changeFlags);
		virtual void PushArguments(lua_State *l) override;
		TransformChangeFlags changeFlags;
	};
	class DLLNETWORK BaseTransformComponent
		: public BaseEntityComponent
	{
	public:
		static pragma::ComponentEventId EVENT_ON_POSE_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);
		virtual void Initialize() override;

		void SetPosition(const Vector3 &pos);
		const Vector3 &GetPosition() const;
		void SetRotation(const Quat &q);
		const Quat &GetRotation() const;
		void SetPose(const umath::ScaledTransform &pose);
		void SetPose(const umath::Transform &pose);
		const umath::ScaledTransform &GetPose() const;
		void SetAngles(const EulerAngles &ang);
		void SetPitch(float pitch);
		void SetYaw(float yaw);
		void SetRoll(float roll);
		EulerAngles GetAngles() const;
		float GetPitch() const;
		float GetYaw() const;
		float GetRoll() const;
		Mat4 GetRotationMatrix() const;
		Vector3 GetForward() const;
		Vector3 GetUp() const;
		Vector3 GetRight() const;
		float GetDistance(const Vector3 &p) const;
		float GetDistance(const BaseEntity &ent) const;
		void GetOrientation(Vector3 *forward,Vector3 *right,Vector3 *up=nullptr) const;

		void LocalToWorld(Vector3 *origin) const;
		void LocalToWorld(Quat *rot) const;
		void LocalToWorld(Vector3 *origin,Quat *rot) const;

		void WorldToLocal(Vector3 *origin) const;
		void WorldToLocal(Quat *rot) const;
		void WorldToLocal(Vector3 *origin,Quat *rot) const;

		Vector3 GetOrigin() const;

		Vector3 GetDirection(const BaseEntity &ent,bool bIgnoreYAxis=false) const;
		EulerAngles GetAngles(const BaseEntity &ent,bool bIgnoreYAxis=false) const;
		float GetDotProduct(const BaseEntity &ent,bool bIgnoreYAxis=false) const;
		Vector3 GetDirection(const Vector3 &pos,bool bIgnoreYAxis=false) const;
		EulerAngles GetAngles(const Vector3 &pos,bool bIgnoreYAxis=false) const;
		float GetDotProduct(const Vector3 &pos,bool bIgnoreYAxis=false) const;

		float GetMaxAxisScale() const;
		float GetAbsMaxAxisScale() const;
		const Vector3 &GetScale() const;
		void SetScale(float scale);
		virtual void SetScale(const Vector3 &scale);

		virtual Vector3 GetEyePosition() const;
		Vector3 GetEyeOffset() const;
		virtual void SetEyeOffset(const Vector3 &offset);

		double GetLastMoveTime() const;
		
		void SetPosition(const Vector3 &pos,Bool bForceUpdate);

		virtual void Save(udm::LinkedPropertyWrapper &udm) override;
		virtual void Load(udm::LinkedPropertyWrapper &udm,uint32_t version) override;

		// Same as SetPosition / SetRotation / SetScale, but don't invoke callbacks
		void SetRawPosition(const Vector3 &pos);
		void SetRawRotation(const Quat &rot);
		void SetRawScale(const Vector3 &scale);

		void UpdateLastMovedTime();
		void OnPoseChanged(TransformChangeFlags changeFlags,bool updatePhysics=true);
	protected:
		BaseTransformComponent(BaseEntity &ent);
		pragma::NetEventId m_netEvSetScale = pragma::INVALID_NET_EVENT;
		double m_tLastMoved = 0.0; // Last time the entity moved or changed rotation
		Vector3 m_eyeOffset = {};
		umath::ScaledTransform m_pose {};
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::TransformChangeFlags)

#endif
