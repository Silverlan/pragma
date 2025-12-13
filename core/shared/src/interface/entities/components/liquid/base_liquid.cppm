// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.liquid.base_liquid;

export import :entities.components.base;
export import :entities.components.base_surface;
export import :entities.components.liquid.base_surface_simulation;
export import :physics.enums;

export namespace pragma {
	class DLLNETWORK BaseFuncLiquidComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		using BaseEntityComponent::BaseEntityComponent;
		struct LocalRayResult {
			float fraction;
			float friction;
			Vector3 hitNormalLocal;
		};

		virtual void Initialize() override;

		bool CalcLineSurfaceIntersection(const Vector3 &lineOrigin, const Vector3 &lineDir, double *outT = nullptr, double *outU = nullptr, double *outV = nullptr, bool bCull = false) const;
		virtual bool OnRayResultCallback(physics::CollisionMask rayCollisionGroup, physics::CollisionMask rayCollisionMask);
		virtual void OnEntitySpawn() override;
	  protected:
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void ClearWaterSurface();
		void InitializeWaterSurface();

		ComponentHandle<BaseSurfaceComponent> m_surfaceC {};
		ComponentHandle<BaseLiquidSurfaceSimulationComponent> m_surfSim {};
	};
};
