/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_ENV_LIGHT_POINT_H__
#define __S_ENV_LIGHT_POINT_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/lights/s_env_light.h"
#include "pragma/entities/environment/lights/env_light_point.h"

namespace pragma
{
	class DLLSERVER SLightPointComponent final
		: public BaseEnvLightPointComponent
	{
	public:
		SLightPointComponent(BaseEntity &ent) : BaseEnvLightPointComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	protected:
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
	};
};

class DLLSERVER EnvLightPoint
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif