// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "sharedutils/netpacket.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include "pragma/entities/environment/env_quake.h"

export module pragma.server.entities.components.quake;

import pragma.server.entities;
import pragma.server.entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SQuakeComponent final : public BaseEnvQuakeComponent, public SBaseNetComponent {
		public:
			SQuakeComponent(BaseEntity &ent) : BaseEnvQuakeComponent(ent) {}
			virtual void Initialize() override;

			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER EnvQuake : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
