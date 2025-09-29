// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "sharedutils/netpacket.hpp"

export module pragma.server.entities.components.effects.smoke_trail;

import pragma.server.entities;
import pragma.server.entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SSmokeTrailComponent final : public BaseEnvSmokeTrailComponent, public SBaseNetComponent {
		public:
			SSmokeTrailComponent(BaseEntity &ent) : BaseEnvSmokeTrailComponent(ent) {}
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER EnvSmokeTrail : public SBaseEntity {
	protected:
	public:
		virtual void Initialize() override;
	};
};
