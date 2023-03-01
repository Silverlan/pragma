/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

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
