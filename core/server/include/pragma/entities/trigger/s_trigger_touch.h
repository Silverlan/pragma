/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_TRIGGER_TOUCH_H__
#define __S_TRIGGER_TOUCH_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include <pragma/entities/components/basetoggle.h>
#include <pragma/entities/trigger/base_trigger_touch.hpp>

namespace pragma
{
	class DLLSERVER STouchComponent final
		: public BaseTouchComponent
	{
	public:
		STouchComponent(BaseEntity &ent) : BaseTouchComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER TriggerTouch
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif