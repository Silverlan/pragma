// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_ENV_SOUND_DSP_DISTORTION_H__
#define __S_ENV_SOUND_DSP_DISTORTION_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp.h"
#include "pragma/entities/environment/audio/env_sound_dsp_distortion.h"

namespace pragma {
	class DLLSERVER SSoundDspDistortionComponent final : public SBaseSoundDspComponent, public BaseEnvSoundDspDistortion {
	  public:
		SSoundDspDistortionComponent(BaseEntity &ent) : SBaseSoundDspComponent(ent) {}
		virtual bool OnSetKeyValue(const std::string &key, const std::string &val) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER EnvSoundDspDistortion : public SBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
