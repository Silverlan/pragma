/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_ENV_SOUND_DSP_FLANGER_H__
#define __S_ENV_SOUND_DSP_FLANGER_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp.h"
#include "pragma/entities/environment/audio/env_sound_dsp_flanger.h"

namespace pragma
{
	class DLLSERVER SSoundDspFlangerComponent final
		: public SBaseSoundDspComponent,
		public BaseEnvSoundDspFlanger
	{
	public:
		SSoundDspFlangerComponent(BaseEntity &ent) : SBaseSoundDspComponent(ent) {}
		virtual bool OnSetKeyValue(const std::string &key,const std::string &val) override;
		virtual void SendData(NetPacket &packet,networking::ClientRecipientFilter &rp) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER EnvSoundDspFlanger
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif