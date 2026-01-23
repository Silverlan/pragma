// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.audio.sound;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SSoundComponent final : public BaseEnvSoundComponent, public SBaseNetComponent {
		  public:
			SSoundComponent(ecs::BaseEntity &ent) : BaseEnvSoundComponent(ent) {}
			virtual void Initialize() override;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua::State *l) override;
		  protected:
			virtual void OnSoundCreated(audio::ALSound &snd) override;
		};
	};

	class DLLSERVER EnvSound : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
