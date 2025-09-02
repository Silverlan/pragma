// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/audio/env_sound_dsp_eaxreverb.h"

export module pragma.server.entities.components.audio.dsp.eaxreverb;

import pragma.server.entities.components.audio.dsp.base;

export {
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
};
