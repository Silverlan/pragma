// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/environment/audio/env_sound_dsp_eaxreverb.h"

export module pragma.client.entities.components.audio.dsp.eaxreverb;

import pragma.client.entities.components.audio.dsp.base;

export namespace pragma {
	class DLLCLIENT CSoundDspEAXReverbComponent final : public CBaseSoundDspComponent, public BaseEnvSoundDspEAXReverb {
	  public:
		CSoundDspEAXReverbComponent(BaseEntity &ent) : CBaseSoundDspComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void OnEntitySpawn() override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

export class DLLCLIENT CEnvSoundDspEAXReverb : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
