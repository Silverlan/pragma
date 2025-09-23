// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/environment/audio/env_soundscape.h"

export module pragma.client:entities.components.audio.sound_scape;

import :entities.base_entity;
import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CSoundScapeComponent final : public BaseEnvSoundScapeComponent, public CBaseNetComponent {
	  public:
		CSoundScapeComponent(BaseEntity &ent) : BaseEnvSoundScapeComponent(ent) {}
		virtual void Initialize() override;
		virtual void OnTick(double dt) override;
		virtual ~CSoundScapeComponent() override;
		virtual void ReceiveData(NetPacket &packet) override;
		void StartSoundScape();
		void StopSoundScape();
		bool IsPlayerInRange();
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void OnEntitySpawn() override;
	  protected:
		std::shared_ptr<ALSound> m_sound;
		static CSoundScapeComponent *s_active;
		std::unordered_map<unsigned int, EntityHandle> m_targets;
		void UpdateTargetPositions();
	};
};

export class DLLCLIENT CEnvSoundScape : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
