/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __BASE_LIQUID_CONTROL_COMPONENT_HPP__
#define __BASE_LIQUID_CONTROL_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/physics/phys_liquid.hpp"
#include "pragma/math/surfacematerial.h"

struct BulletInfo;
namespace pragma {
	class DLLNETWORK BaseLiquidControlComponent : public BaseEntityComponent {
	  public:
		struct DLLNETWORK SplashInfo {
			Vector3 origin = {};
			float radius = 0.f;
			float force = 0.f;
		};

		static ComponentEventId EVENT_ON_SPLASH;
		static ComponentEventId EVENT_ON_PROPERTIES_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;

		const Vector3 &GetLiquidVelocity() const;
		void SetLiquidVelocity(const Vector3 &velocity);

		const PhysLiquid &GetLiquidDescription() const;
		PhysLiquid &GetLiquidDescription();

		double GetDensity() const;
		void SetDensity(double density);

		double GetLinearDragCoefficient() const;
		void SetLinearDragCoefficient(double coefficient);

		double GetTorqueDragCoefficient() const;
		void SetTorqueDragCoefficient(double coefficient);

		float GetStiffness() const;
		virtual void SetStiffness(float stiffness);
		float GetPropagation() const;
		virtual void SetPropagation(float propagation);

		virtual void CreateSplash(const Vector3 &origin, float radius, float force);
	  protected:
		BaseLiquidControlComponent(BaseEntity &ent);
		struct LocalRayResult {
			float fraction;
			float friction;
			Vector3 hitNormalLocal;
		};
		virtual bool OnBulletHit(const BulletInfo &bulletInfo, const TraceData &data, PhysObj *phys, pragma::physics::ICollisionObject *col, const LocalRayResult &result);
		void SetSurfaceMaterial(const std::string &mat);
		void SetSurfaceMaterial(const SurfaceMaterial *mat);
		Vector3 m_liquidVelocity;
		PhysLiquid m_liquid = {};
		std::string m_kvSurfaceMaterial;
		pragma::NetEventId m_netEvCreateSplash = pragma::INVALID_NET_EVENT;

		std::queue<SplashInfo> m_splashes;
	};
	struct DLLNETWORK CEOnSplash : public ComponentEvent {
		CEOnSplash(const BaseLiquidControlComponent::SplashInfo &splashInfo);
		virtual void PushArguments(lua_State *l) override;
		const BaseLiquidControlComponent::SplashInfo &splashInfo;
	};
};

#endif
