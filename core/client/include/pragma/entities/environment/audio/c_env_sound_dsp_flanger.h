/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
