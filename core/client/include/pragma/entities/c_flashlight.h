/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_FLASHLIGHT_H__
#define __C_FLASHLIGHT_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/baseflashlight.h>
#include "pragma/entities/environment/lights/c_env_light_spot.h"

namespace pragma
{
	class DLLCLIENT CFlashlightComponent final
		: public BaseFlashlightComponent
	{
	public:
		CFlashlightComponent(BaseEntity &ent) : BaseFlashlightComponent(ent) {}
		virtual void Initialize() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class CEnvLightSpot;
class DLLCLIENT CFlashlight
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif