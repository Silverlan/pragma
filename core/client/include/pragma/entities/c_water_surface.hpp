/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_WATER_SURFACE_HPP__
#define __C_WATER_SURFACE_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"

class PhysWaterSurfaceSimulator;
namespace pragma {
	class CLiquidSurfaceSimulationComponent;
	class DLLCLIENT CWaterSurfaceComponent final : public BaseEntityComponent {
	  public:
		CWaterSurfaceComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;

		virtual ~CWaterSurfaceComponent() override;
		void SetSurfaceSimulator(const std::shared_ptr<PhysWaterSurfaceSimulator> &simulator);
		CMaterial *GetWaterMaterial() const;
		void SetWaterObject(CLiquidSurfaceSimulationComponent *ent);
		CModelSubMesh *GetWaterSurfaceMesh() const;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnEntitySpawn() override;
	  protected:
		std::shared_ptr<PhysWaterSurfaceSimulator> m_surfaceSimulator = nullptr;
		mutable std::weak_ptr<CModelSubMesh> m_waterSurfaceMesh = {};
		CallbackHandle m_cbRenderSurface = {};
		ComponentHandle<CLiquidSurfaceSimulationComponent> m_hFuncWater = {};
		void UpdateSurfaceMesh();
		void InitializeSurface();
		void DestroySurface();
	};
};

class PhysWaterSurfaceSimulator;
class CFuncWater;
class DLLCLIENT CWaterSurface : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
