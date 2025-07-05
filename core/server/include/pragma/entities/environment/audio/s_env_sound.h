// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_ENV_SOUND_H__
#define __S_ENV_SOUND_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/audio/env_sound.h"
#include "pragma/entities/components/s_entity_component.hpp"

namespace pragma {
	class DLLSERVER SSoundComponent final : public BaseEnvSoundComponent, public SBaseNetComponent {
	  public:
		SSoundComponent(BaseEntity &ent) : BaseEnvSoundComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		virtual void OnSoundCreated(ALSound &snd) override;
	};
};

class DLLSERVER EnvSound : public SBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
