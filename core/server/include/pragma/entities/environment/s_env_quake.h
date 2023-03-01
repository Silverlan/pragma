/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_ENV_QUAKE_H__
#define __S_ENV_QUAKE_H__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/env_quake.h"
#include "pragma/entities/components/s_entity_component.hpp"

namespace pragma {
	class DLLSERVER SQuakeComponent final : public BaseEnvQuakeComponent, public SBaseNetComponent {
	  public:
		SQuakeComponent(BaseEntity &ent) : BaseEnvQuakeComponent(ent) {}
		virtual void Initialize() override;

		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER EnvQuake : public SBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
