// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/physics/phys_water_surface_simulator.hpp"
#include "pragma/entities/components/liquid/base_liquid_surface_simulation_component.hpp"
#include "pragma/entities/components/liquid/c_liquid_component.hpp"
#include <cmaterial.h>

export module pragma.client.entities.components.water_surface;

export namespace pragma {
	class DLLCLIENT CWaterSurfaceComponent final : public BaseEntityComponent {
	  public:
		CWaterSurfaceComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;

		virtual ~CWaterSurfaceComponent() override;
		void SetSurfaceSimulator(const std::shared_ptr<PhysWaterSurfaceSimulator> &simulator);
		CMaterial *GetWaterMaterial() const;
		void SetWaterObject(BaseLiquidSurfaceSimulationComponent *ent);
		CModelSubMesh *GetWaterSurfaceMesh() const;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnEntitySpawn() override;
	  protected:
		std::shared_ptr<PhysWaterSurfaceSimulator> m_surfaceSimulator = nullptr;
		mutable std::weak_ptr<CModelSubMesh> m_waterSurfaceMesh = {};
		CallbackHandle m_cbRenderSurface = {};
		ComponentHandle<BaseLiquidSurfaceSimulationComponent> m_hFuncWater = {};
		void UpdateSurfaceMesh();
		void InitializeSurface();
		void DestroySurface();
	};
};

export class DLLCLIENT CWaterSurface : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
