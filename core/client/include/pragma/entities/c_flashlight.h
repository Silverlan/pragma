// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_FLASHLIGHT_H__
#define __C_FLASHLIGHT_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/baseflashlight.h>
#include "pragma/entities/environment/lights/c_env_light_spot.h"

namespace pragma {
	class DLLCLIENT CFlashlightComponent final : public BaseFlashlightComponent {
	  public:
		CFlashlightComponent(BaseEntity &ent) : BaseFlashlightComponent(ent) {}
		virtual void Initialize() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class CEnvLightSpot;
class DLLCLIENT CFlashlight : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
