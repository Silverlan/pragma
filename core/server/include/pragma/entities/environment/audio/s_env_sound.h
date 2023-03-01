/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

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
