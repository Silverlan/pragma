// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/entities/environment/audio/env_soundscape.h"
#include "pragma/entities/components/s_entity_component.hpp"

export module pragma.server.entities.components.audio.soundscape;

import pragma.server.entities;

export {
	namespace pragma {
		class DLLSERVER SSoundScapeComponent final : public BaseEnvSoundScapeComponent, public SBaseNetComponent {
		public:
			SSoundScapeComponent(BaseEntity &ent) : BaseEnvSoundScapeComponent(ent) {}
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER EnvSoundScape : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
