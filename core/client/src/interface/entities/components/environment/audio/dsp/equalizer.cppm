// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"


export module pragma.client:entities.components.audio.dsp.equalizer;
import :entities.components.audio.dsp.base;

export namespace pragma {
	class DLLCLIENT CSoundDspEqualizerComponent final : public CBaseSoundDspComponent, public BaseEnvSoundDspEqualizer {
	  public:
		CSoundDspEqualizerComponent(BaseEntity &ent) : CBaseSoundDspComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void OnEntitySpawn() override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

export class DLLCLIENT CEnvSoundDspEqualizer : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
