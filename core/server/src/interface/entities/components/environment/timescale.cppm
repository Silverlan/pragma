// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "sharedutils/netpacket.hpp"

export module pragma.server.entities.components.timescale;

import pragma.server.entities;
import pragma.server.entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SEnvTimescaleComponent final : public BaseEnvTimescaleComponent, public SBaseNetComponent {
		public:
			SEnvTimescaleComponent(BaseEntity &ent) : BaseEnvTimescaleComponent(ent) {}
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER EnvTimescale : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
