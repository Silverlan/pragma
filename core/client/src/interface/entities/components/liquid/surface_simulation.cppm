// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.liquid_surface_simulation;

export import :entities.components.entity;
export import :entities.components.water_surface;

export namespace pragma {
	class DLLCLIENT CLiquidSurfaceSimulationComponent final : public BaseLiquidSurfaceSimulationComponent, public CBaseNetComponent {
	  public:
		CLiquidSurfaceSimulationComponent(ecs::BaseEntity &ent);
		virtual ~CLiquidSurfaceSimulationComponent() override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void ReloadSurfaceSimulator() override;
		CWaterSurface *GetSurfaceEntity() const;
	  protected:
		virtual void OnEntitySpawn() override;
		virtual std::shared_ptr<physics::PhysWaterSurfaceSimulator> InitializeSurfaceSimulator(const Vector2 &min, const Vector2 &max, float originY) override;
		mutable EntityHandle m_hWaterSurface = {};
	};
};
