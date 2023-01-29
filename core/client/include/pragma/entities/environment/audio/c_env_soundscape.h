/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_ENV_SOUNDSCAPE_H__
#define __C_ENV_SOUNDSCAPE_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/audio/env_soundscape.h"

namespace pragma {
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

class DLLCLIENT CEnvSoundScape : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
