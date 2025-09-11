// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "sharedutils/netpacket.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include <pragma/entities/components/liquid/base_liquid_surface_simulation_component.hpp>

export module pragma.server.entities.components.liquid_surface_simulation;

import pragma.server.entities.components.entity;

export namespace pragma {
	class DLLSERVER SLiquidSurfaceSimulationComponent final : public BaseLiquidSurfaceSimulationComponent, public SBaseNetComponent {
	  public:
		SLiquidSurfaceSimulationComponent(BaseEntity &ent);
		virtual ~SLiquidSurfaceSimulationComponent() override;
		virtual void Initialize() override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnTick(double dt) override;
		void UpdateSurfaceSimulator();
	  protected:
		bool m_bUsingClientsideSimulation = false;
		CallbackHandle m_cbClientSimulatorUpdate = {};
		virtual bool ShouldSimulateSurface() const override;
	};
};
