// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/lua/luaapi.h"


export module pragma.server.entities.components.lights.base;

import pragma.server.entities;
import pragma.server.entities.components.entity;

export namespace pragma {
	class DLLSERVER SLightComponent final : public BaseEnvLightComponent, public SBaseNetComponent {
	  public:
		SLightComponent(BaseEntity &ent) : BaseEnvLightComponent(ent) {}
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void SetShadowType(ShadowType type) override;
		virtual void InitializeLuaObject(lua_State *l) override;

		virtual void SetFalloffExponent(float falloffExponent) override;
	};
};
