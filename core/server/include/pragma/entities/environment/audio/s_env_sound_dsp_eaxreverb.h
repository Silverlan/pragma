/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_ENV_SOUND_DSP_EAXREVERB_H__
#define __S_ENV_SOUND_DSP_EAXREVERB_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp.h"
#include "pragma/entities/environment/audio/env_sound_dsp_eaxreverb.h"

namespace pragma {
	class DLLSERVER SSoundDspEAXReverbComponent final : public SBaseSoundDspComponent, public BaseEnvSoundDspEAXReverb {
	  public:
		SSoundDspEAXReverbComponent(BaseEntity &ent) : SBaseSoundDspComponent(ent) {}
		virtual bool OnSetKeyValue(const std::string &key, const std::string &val) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER EnvSoundDspEAXReverb : public SBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
