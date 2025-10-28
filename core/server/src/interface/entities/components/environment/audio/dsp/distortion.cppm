// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"


export module pragma.server.entities.components.audio.dsp.distortion;

import pragma.server.entities;
import pragma.server.entities.components.audio.dsp.base;

export {
	namespace pragma {
		class DLLSERVER SSoundDspDistortionComponent final : public SBaseSoundDspComponent, public BaseEnvSoundDspDistortion {
		public:
			SSoundDspDistortionComponent(pragma::ecs::BaseEntity &ent) : SBaseSoundDspComponent(ent) {}
			virtual bool OnSetKeyValue(const std::string &key, const std::string &val) override;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER EnvSoundDspDistortion : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
