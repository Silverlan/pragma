/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_ENV_SOUND_DSP_DISTORTION_H__
#define __S_ENV_SOUND_DSP_DISTORTION_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp.h"
#include "pragma/entities/environment/audio/env_sound_dsp_distortion.h"

namespace pragma
{
	class DLLSERVER SSoundDspDistortionComponent final
		: public SBaseSoundDspComponent,
		public BaseEnvSoundDspDistortion
	{
	public:
		SSoundDspDistortionComponent(BaseEntity &ent) : SBaseSoundDspComponent(ent) {}
		virtual bool OnSetKeyValue(const std::string &key,const std::string &val) override;
		virtual void SendData(NetPacket &packet,networking::ClientRecipientFilter &rp) override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER EnvSoundDspDistortion
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif