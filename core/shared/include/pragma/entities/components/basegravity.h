/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASEGRAVITY_H__
#define __BASEGRAVITY_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include <mathutil/glmutil.h>

class NetworkState;
namespace pragma
{
	class DLLNETWORK BaseGravity
	{
	public:
		virtual ~BaseGravity()=default;
		virtual void SetGravityScale(float scale);
		void SetGravityOverride(const Vector3 &dir,float gravity);
		virtual void SetGravityOverride(const Vector3 &dir);
		virtual void SetGravityOverride(float gravity);
		virtual void SetGravityOverride();
		float GetGravityScale() const;
		bool HasGravityForceOverride() const;
		bool HasGravityDirectionOverride() const;
		virtual Vector3 GetGravityDirection() const=0;
		virtual float GetGravity() const=0;
		virtual Vector3 GetGravityForce() const=0;
	protected:
		float m_gravityScale = 1.f;
		std::shared_ptr<Vector3> m_gravityDir = nullptr;
		std::shared_ptr<float> m_gravity = nullptr;
		Vector3 GetGravityForce(NetworkState *state) const;
		Vector3 GetGravityDirection(NetworkState *state) const;
		float GetGravity(NetworkState *state) const;
	};

	class DLLNETWORK GravityComponent final
		: public BaseEntityComponent,
		public BaseGravity
	{
	public:
		GravityComponent(BaseEntity &ent);
		virtual void Initialize() override;

		virtual void SetGravityScale(float scale) override;
		using BaseGravity::SetGravityOverride;
		virtual void SetGravityOverride(const Vector3 &dir) override;
		virtual void SetGravityOverride(float gravity) override;
		virtual void SetGravityOverride() override;

		virtual Vector3 GetGravityDirection() const override;
		virtual float GetGravity() const override;
		virtual Vector3 GetGravityForce() const override;

		bool CalcBallisticVelocity(const Vector3 &origin,const Vector3 &destPos,float fireAngle,float maxSpeed,float spread,float maxPitch,float maxYaw,Vector3 &vel) const;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	private:
		void OnPhysicsInitialized();
		void ApplyGravity(double dt);
	};
};

#endif
