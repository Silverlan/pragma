// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/env_wind.hpp"
#include "pragma/entities/components/s_entity_component.hpp"

export module pragma.server.entities.components.wind;

export {
	namespace pragma {
		class DLLSERVER SWindComponent final : public BaseEnvWindComponent, public SBaseNetComponent {
		public:
			SWindComponent(BaseEntity &ent) : BaseEnvWindComponent(ent) {}
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER EnvWind : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
