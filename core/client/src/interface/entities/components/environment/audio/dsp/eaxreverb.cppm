// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"



export module pragma.client:entities.components.audio.dsp.eaxreverb;
import :entities.components.audio.dsp.base;

export namespace pragma {
	class DLLCLIENT CSoundDspEAXReverbComponent final : public CBaseSoundDspComponent, public BaseEnvSoundDspEAXReverb {
	  public:
		CSoundDspEAXReverbComponent(pragma::ecs::BaseEntity &ent) : CBaseSoundDspComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void OnEntitySpawn() override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

export class DLLCLIENT CEnvSoundDspEAXReverb : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
