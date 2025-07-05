// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_ENV_SOUND_H__
#define __C_ENV_SOUND_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/audio/env_sound.h"
#include "pragma/entities/components/c_entity_component.hpp"

namespace pragma {
	class DLLCLIENT CSoundComponent final : public BaseEnvSoundComponent, public CBaseNetComponent {
	  public:
		CSoundComponent(BaseEntity &ent) : BaseEnvSoundComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		float GetMaxDistance() const;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void OnEntitySpawn() override;
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	  public:
		const std::string &GetSteamAudioIdentifier() const;
	  protected:
		std::string m_steamAudioIdentifier;
#endif
	  protected:
		float m_kvMaxDistance = 0.f;
		std::weak_ptr<ALSound> m_wpSound = {};
	};
};

class DLLCLIENT CEnvSound : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
