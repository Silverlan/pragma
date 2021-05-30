/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_ENV_SOUNDSCAPE_H__
#define __S_ENV_SOUNDSCAPE_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/audio/env_soundscape.h"
#include "pragma/entities/components/s_entity_component.hpp"

namespace pragma
{
	class DLLSERVER SSoundScapeComponent final
		: public BaseEnvSoundScapeComponent,
		public SBaseNetComponent
	{
	public:
		SSoundScapeComponent(BaseEntity &ent) : BaseEnvSoundScapeComponent(ent) {}
		virtual void SendData(NetPacket &packet,networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER EnvSoundScape
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif