/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_TRIGGER_HURT_H__
#define __S_TRIGGER_HURT_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/trigger/base_trigger_hurt.hpp"

namespace pragma
{
	class DLLSERVER STriggerHurtComponent final
		: public BaseTriggerHurtComponent
	{
	public:
		STriggerHurtComponent(BaseEntity &ent) : BaseTriggerHurtComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER TriggerHurt
	: public SBaseEntity
{
protected:
public:
	virtual void Initialize() override;
};

#endif