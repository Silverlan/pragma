// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.water_surface;

export import :entities.base_entity;
import :model.mesh;

export namespace pragma {
	class DLLCLIENT CWaterSurfaceComponent final : public BaseEntityComponent {
	  public:
		CWaterSurfaceComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;

		virtual ~CWaterSurfaceComponent() override;
		void SetSurfaceSimulator(const std::shared_ptr<physics::PhysWaterSurfaceSimulator> &simulator);
		material::CMaterial *GetWaterMaterial() const;
		void SetWaterObject(BaseLiquidSurfaceSimulationComponent *ent);
		geometry::CModelSubMesh *GetWaterSurfaceMesh() const;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void OnEntitySpawn() override;
	  protected:
		std::shared_ptr<physics::PhysWaterSurfaceSimulator> m_surfaceSimulator = nullptr;
		mutable std::weak_ptr<geometry::CModelSubMesh> m_waterSurfaceMesh = {};
		CallbackHandle m_cbRenderSurface = {};
		ComponentHandle<BaseLiquidSurfaceSimulationComponent> m_hFuncWater = {};
		void UpdateSurfaceMesh();
		void InitializeSurface();
		void DestroySurface();
	};
};

export class DLLCLIENT CWaterSurface : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
