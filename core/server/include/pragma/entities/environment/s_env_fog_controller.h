/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_ENV_FOG_CONTROLLER_H__
#define __S_ENV_FOG_CONTROLLER_H__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/env_fog_controller.h"
#include "pragma/entities/components/s_entity_component.hpp"

namespace pragma
{
	class DLLSERVER SFogControllerComponent final
		: public BaseEnvFogControllerComponent,
		public SBaseNetComponent
	{
	public:
		SFogControllerComponent(BaseEntity &ent) : BaseEnvFogControllerComponent(ent) {}
		virtual void SetFogStart(float start) override;
		virtual void SetFogEnd(float end) override;
		virtual void SetMaxDensity(float density) override;
		virtual void SetFogType(util::FogType type) override;

		virtual void SendData(NetPacket &packet,networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER EnvFogController
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif