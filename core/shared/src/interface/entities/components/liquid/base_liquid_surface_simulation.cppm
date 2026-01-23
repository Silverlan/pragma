// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.liquid.base_surface_simulation;

export import :entities.components.base;
export import :entities.components.base_surface;
export import :physics.water_surface_simulator;

export namespace pragma {
	namespace baseLiquidSurfaceSimulationComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED)
	}
	class DLLNETWORK BaseLiquidSurfaceSimulationComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		enum class DLLNETWORK SpawnFlags : uint32_t { SurfaceSimulation = 2048 };
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;

		uint32_t GetSpacing() const;
		void SetSpacing(uint32_t spacing);

		const physics::PhysWaterSurfaceSimulator *GetSurfaceSimulator() const;
		physics::PhysWaterSurfaceSimulator *GetSurfaceSimulator();

		virtual void ReloadSurfaceSimulator();
		void ClearSurfaceSimulator();

		bool CalcLineSurfaceIntersection(const Vector3 &lineOrigin, const Vector3 &lineDir, double *outT = nullptr, double *outU = nullptr, double *outV = nullptr, bool bCull = false) const;
	  protected:
		BaseLiquidSurfaceSimulationComponent(ecs::BaseEntity &ent);
		BaseSurfaceComponent *GetSurfaceComponent() const;

		virtual std::shared_ptr<physics::PhysWaterSurfaceSimulator> InitializeSurfaceSimulator(const Vector2 &min, const Vector2 &max, float originY);
		virtual bool ShouldSimulateSurface() const;
		void SetMaxWaveHeight(float height);
		float m_kvMaxWaveHeight = 100.f;
		uint32_t m_spacing = 10;
		std::shared_ptr<physics::PhysWaterSurfaceSimulator> m_physSurfaceSim = nullptr;
		double m_originalWaterPlaneDistance = 0.0;
	};
};
