/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_ENV_QUAKE_H__
#define __C_ENV_QUAKE_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/env_quake.h"
#include "pragma/entities/components/c_entity_component.hpp"

namespace pragma
{
	class DLLCLIENT CQuakeComponent final
		: public BaseEnvQuakeComponent,
		public CBaseNetComponent
	{
	public:
		CQuakeComponent(BaseEntity &ent) : BaseEnvQuakeComponent(ent) {}
		virtual ~CQuakeComponent() override;

		virtual void ReceiveData(NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	protected:
		CallbackHandle m_cbScreenShake;
		virtual void StartShake() override;
		void CancelScreenShake();
	};
};

class DLLCLIENT CEnvQuake
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif