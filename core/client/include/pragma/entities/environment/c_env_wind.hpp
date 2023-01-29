/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_ENV_WIND_HPP__
#define __C_ENV_WIND_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/env_wind.hpp"
#include "pragma/entities/components/c_entity_component.hpp"

namespace pragma {
	class DLLCLIENT CWindComponent final : public BaseEnvWindComponent, public CBaseNetComponent {
	  public:
		CWindComponent(BaseEntity &ent) : BaseEnvWindComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};
};

class DLLCLIENT CEnvWind : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
