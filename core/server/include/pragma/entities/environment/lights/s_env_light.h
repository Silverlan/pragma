/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_ENV_LIGHT_H__
#define __S_ENV_LIGHT_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/lights/env_light.h"
#include "pragma/entities/components/s_entity_component.hpp"

namespace pragma
{
	class DLLSERVER SLightComponent final
		: public BaseEnvLightComponent,
		public SBaseNetComponent
	{
	public:
		SLightComponent(BaseEntity &ent) : BaseEnvLightComponent(ent) {}
		virtual void SendData(NetPacket &packet,networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual void SetShadowType(ShadowType type) override;
		virtual void InitializeLuaObject(lua_State *l) override;

		virtual void SetFalloffExponent(float falloffExponent) override;
	};
};

#endif
