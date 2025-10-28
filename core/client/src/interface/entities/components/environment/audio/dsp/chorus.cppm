// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"



export module pragma.client:entities.components.audio.dsp.chorus;
import :entities.components.audio.dsp.base;

export namespace pragma {
	class DLLCLIENT CSoundDspChorusComponent final : public CBaseSoundDspComponent, public BaseEnvSoundDspChorus {
	  public:
		CSoundDspChorusComponent(BaseEntity &ent) : CBaseSoundDspComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void OnEntitySpawn() override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

export class DLLCLIENT CEnvSoundDspChorus : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
