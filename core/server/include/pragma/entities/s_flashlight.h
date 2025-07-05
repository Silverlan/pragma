// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_FLASHLIGHT_H__
#define __S_FLASHLIGHT_H__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include <pragma/entities/baseflashlight.h>

namespace pragma {
	class DLLSERVER SFlashlightComponent final : public BaseFlashlightComponent {
	  public:
		SFlashlightComponent(BaseEntity &ent) : BaseFlashlightComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER Flashlight : public SBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
