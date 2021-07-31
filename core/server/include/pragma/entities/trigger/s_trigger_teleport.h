/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_TRIGGER_TELEPORT_H__
#define __S_TRIGGER_TELEPORT_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/trigger/s_trigger_touch.h"
#include <pragma/entities/trigger/base_trigger_teleport.hpp>

namespace pragma
{
	class DLLSERVER STriggerTeleportComponent final
		: public BaseTriggerTeleportComponent
	{
	public:
		STriggerTeleportComponent(BaseEntity &ent) : BaseTriggerTeleportComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER TriggerTeleport
	: public TriggerTouch
{
public:
	virtual void Initialize() override;

};

#endif