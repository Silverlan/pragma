/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __BASE_TRANSFORM_COMPONENT_HPP__
#define __BASE_TRANSFORM_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property_vector.h>
#include <sharedutils/property/util_property_quat.hpp>

namespace pragma
{
	class DLLNETWORK BaseTransformComponent
		: public BaseEntityComponent
	{
	public:
		virtual void Initialize() override;

		virtual void SetPosition(const Vector3 &pos);
		virtual const Vector3 &GetPosition() const;
		virtual void SetOrientation(const Quat &q);
		virtual const Quat &GetOrientation() const;
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

		const util::PVector3Property &GetPosProperty() const;
		const util::PQuatProperty &GetOrientationProperty() const;
		const util::PVector3Property &GetScaleProperty() const;

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

		virtual void Save(DataStream &ds) override;
		virtual void Load(DataStream &ds,uint32_t version) override;

		void UpdateLastMovedTime();

		// Set member variables directly, without influencing physics
		void SetRawPosition(const Vector3 &pos);
		void SetRawOrientation(const Quat &rot);
		void SetRawScale(const Vector3 &scale);
	protected:
		BaseTransformComponent(BaseEntity &ent);
		pragma::NetEventId m_netEvSetScale = pragma::INVALID_NET_EVENT;
		double m_tLastMoved = 0.0; // Last time the entity moved or changed rotation
		Vector3 m_eyeOffset = {};
		util::PVector3Property m_pos;
		util::PQuatProperty m_orientation;
		util::PVector3Property m_scale;
	};
};

#endif
