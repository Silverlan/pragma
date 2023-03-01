/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __BASE_LIQUID_SURFACE_SIMULATION_COMPONENT_HPP__
#define __BASE_LIQUID_SURFACE_SIMULATION_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

class PhysWaterSurfaceSimulator;
namespace pragma {
	class BaseSurfaceComponent;
	class DLLNETWORK BaseLiquidSurfaceSimulationComponent : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		enum class DLLNETWORK SpawnFlags : uint32_t { SurfaceSimulation = 2048 };
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;

		uint32_t GetSpacing() const;
		void SetSpacing(uint32_t spacing);

		const PhysWaterSurfaceSimulator *GetSurfaceSimulator() const;
		PhysWaterSurfaceSimulator *GetSurfaceSimulator();

		virtual void ReloadSurfaceSimulator();
		void ClearSurfaceSimulator();

		bool CalcLineSurfaceIntersection(const Vector3 &lineOrigin, const Vector3 &lineDir, double *outT = nullptr, double *outU = nullptr, double *outV = nullptr, bool bCull = false) const;
	  protected:
		BaseLiquidSurfaceSimulationComponent(BaseEntity &ent);
		BaseSurfaceComponent *GetSurfaceComponent() const;

		virtual std::shared_ptr<PhysWaterSurfaceSimulator> InitializeSurfaceSimulator(const Vector2 &min, const Vector2 &max, float originY);
		virtual bool ShouldSimulateSurface() const;
		void SetMaxWaveHeight(float height);
		float m_kvMaxWaveHeight = 100.f;
		uint32_t m_spacing = 10;
		std::shared_ptr<PhysWaterSurfaceSimulator> m_physSurfaceSim = nullptr;
		double m_originalWaterPlaneDistance = 0.0;
	};
};

#endif
