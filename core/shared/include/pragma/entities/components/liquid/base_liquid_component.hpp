/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASEFUNCWATER_H__
#define __BASEFUNCWATER_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/physics/phys_liquid.hpp"
#include "pragma/entities/trigger/base_trigger_touch.hpp"

struct BulletInfo;
namespace pragma {
	class BaseSurfaceComponent;
	class BaseLiquidSurfaceSimulationComponent;
	class DLLNETWORK BaseFuncLiquidComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		using BaseEntityComponent::BaseEntityComponent;
		struct LocalRayResult {
			float fraction;
			float friction;
			Vector3 hitNormalLocal;
		};

		virtual void Initialize() override;

		bool CalcLineSurfaceIntersection(const Vector3 &lineOrigin, const Vector3 &lineDir, double *outT = nullptr, double *outU = nullptr, double *outV = nullptr, bool bCull = false) const;
		virtual bool OnRayResultCallback(CollisionMask rayCollisionGroup, CollisionMask rayCollisionMask);
		virtual void OnEntitySpawn() override;
	  protected:
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void ClearWaterSurface();
		void InitializeWaterSurface();

		pragma::ComponentHandle<BaseSurfaceComponent> m_surfaceC {};
		pragma::ComponentHandle<BaseLiquidSurfaceSimulationComponent> m_surfSim {};
	};
};

#endif
