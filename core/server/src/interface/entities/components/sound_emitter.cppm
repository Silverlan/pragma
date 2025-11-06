// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.server:entities.components.sound_emitter;

import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SSoundEmitterComponent final : public BaseSoundEmitterComponent, public SBaseNetComponent {
	  public:
		SSoundEmitterComponent(pragma::ecs::BaseEntity &ent) : BaseSoundEmitterComponent(ent) {}
		virtual void UpdateSoundTransform(ALSound &snd) const override;
		virtual std::shared_ptr<ALSound> CreateSound(std::string sndname, pragma::audio::ALSoundType type, const SoundInfo &sndInfo = {}) override;
		virtual std::shared_ptr<ALSound> EmitSound(std::string sndname, pragma::audio::ALSoundType type, const SoundInfo &sndInfo = {}) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};
