// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.audio.soundscape;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SSoundScapeComponent final : public BaseEnvSoundScapeComponent, public SBaseNetComponent {
		  public:
			SSoundScapeComponent(ecs::BaseEntity &ent) : BaseEnvSoundScapeComponent(ent) {}
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER EnvSoundScape : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
