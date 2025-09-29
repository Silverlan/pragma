// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "sharedutils/netpacket.hpp"

export module pragma.server.entities.components.audio.dsp.equalizer;

import pragma.server.entities;
import pragma.server.entities.components.audio.dsp.base;

export {
	namespace pragma {
		class DLLSERVER SSoundDspEqualizerComponent final : public SBaseSoundDspComponent, public BaseEnvSoundDspEqualizer {
		public:
			SSoundDspEqualizerComponent(BaseEntity &ent) : SBaseSoundDspComponent(ent) {}
			virtual bool OnSetKeyValue(const std::string &key, const std::string &val) override;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER EnvSoundDspEqualizer : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
