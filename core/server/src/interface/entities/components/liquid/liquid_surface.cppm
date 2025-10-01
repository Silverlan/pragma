// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"

export module pragma.server.entities.components.liquid_surface;

import pragma.server.entities.components.entity;

export namespace pragma {
	class DLLSERVER SLiquidSurfaceComponent final : public BaseLiquidSurfaceComponent, public SBaseNetComponent {
	  public:
		SLiquidSurfaceComponent(BaseEntity &ent) : BaseLiquidSurfaceComponent(ent) {}
		virtual void Initialize() override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};
