// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.liquid_surface_simulation;

import :entities.components.entity;

export namespace pragma {
	namespace sLiquidSurfaceSimulationComponent {
		using namespace baseLiquidSurfaceSimulationComponent;
	}
	class DLLSERVER SLiquidSurfaceSimulationComponent final : public BaseLiquidSurfaceSimulationComponent, public SBaseNetComponent {
	  public:
		SLiquidSurfaceSimulationComponent(ecs::BaseEntity &ent);
		virtual ~SLiquidSurfaceSimulationComponent() override;
		virtual void Initialize() override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void OnTick(double dt) override;
		void UpdateSurfaceSimulator();
	  protected:
		bool m_bUsingClientsideSimulation = false;
		CallbackHandle m_cbClientSimulatorUpdate = {};
		virtual bool ShouldSimulateSurface() const override;
	};
};
