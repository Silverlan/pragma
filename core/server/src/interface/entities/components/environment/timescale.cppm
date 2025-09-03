// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/env_timescale.h"
#include "pragma/entities/components/s_entity_component.hpp"

export module pragma.server.entities.components.timescale;

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
