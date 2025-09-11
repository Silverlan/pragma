// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "sharedutils/netpacket.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include "pragma/entities/environment/effects/env_particle_system.h"

export module pragma.server.entities.components.effects.particle_system;

import pragma.server.entities;
import pragma.server.entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SParticleSystemComponent final : public BaseEnvParticleSystemComponent, public SBaseNetComponent {
		public:
			SParticleSystemComponent(BaseEntity &ent) : BaseEnvParticleSystemComponent(ent) {}
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual void SetContinuous(bool b) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER EnvParticleSystem : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
