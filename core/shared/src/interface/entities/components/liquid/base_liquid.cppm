// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/physics/phys_liquid.hpp"
#include "pragma/entities/trigger/base_trigger_touch.hpp"

export module pragma.shared:entities.components.liquid.base_liquid;

export namespace pragma {
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
