// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
