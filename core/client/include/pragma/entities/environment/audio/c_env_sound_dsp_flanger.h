// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_ENV_SOUND_DSP_FLANGER_H__
#define __C_ENV_SOUND_DSP_FLANGER_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp.h"
#include "pragma/entities/environment/audio/env_sound_dsp_flanger.h"

namespace pragma {
	class DLLCLIENT CSoundDspFlangerComponent final : public CBaseSoundDspComponent, public BaseEnvSoundDspFlanger {
	  public:
		CSoundDspFlangerComponent(BaseEntity &ent) : CBaseSoundDspComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void OnEntitySpawn() override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLCLIENT CEnvSoundDspFlanger : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
