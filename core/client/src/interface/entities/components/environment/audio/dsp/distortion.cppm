// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/audio/env_sound_dsp_distortion.h"

export module pragma.client.entities.components.audio.dsp.distortion;

import pragma.client.entities.components.audio.dsp.base;

export namespace pragma {
	class DLLCLIENT CSoundDspDistortionComponent final : public CBaseSoundDspComponent, public BaseEnvSoundDspDistortion {
	  public:
		CSoundDspDistortionComponent(BaseEntity &ent) : CBaseSoundDspComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void OnEntitySpawn() override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

export class DLLCLIENT CEnvSoundDspDistortion : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
