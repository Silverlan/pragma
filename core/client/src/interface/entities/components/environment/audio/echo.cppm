// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp.h"
#include "pragma/entities/environment/audio/env_sound_dsp_echo.h"

export module pragma.client.entities.components.audio.dsp.echo;

export namespace pragma {
	class DLLCLIENT CSoundDspEchoComponent final : public CBaseSoundDspComponent, public BaseEnvSoundDspEcho {
	  public:
		CSoundDspEchoComponent(BaseEntity &ent) : CBaseSoundDspComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void OnEntitySpawn() override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

export class DLLCLIENT CEnvSoundDspEcho : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
