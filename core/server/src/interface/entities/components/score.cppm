// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "sharedutils/netpacket.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include <pragma/entities/components/base_score_component.hpp>

export module pragma.server.entities.components.score;

import pragma.server.entities.components.entity;

export namespace pragma {
	class DLLSERVER SScoreComponent final : public BaseScoreComponent, public SBaseNetComponent {
	  public:
		SScoreComponent(BaseEntity &ent) : BaseScoreComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;

		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};
