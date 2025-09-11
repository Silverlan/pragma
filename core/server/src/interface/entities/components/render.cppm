// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "sharedutils/netpacket.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include <pragma/entities/components/base_render_component.hpp>

export module pragma.server.entities.components.render;

import pragma.server.entities.components.entity;

export namespace pragma {
	class DLLSERVER SRenderComponent final : public BaseRenderComponent, public SBaseNetComponent {
	  public:
		SRenderComponent(BaseEntity &ent) : BaseRenderComponent(ent) {}
		virtual void SetUnlit(bool b) override;
		virtual void SetCastShadows(bool b) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};
