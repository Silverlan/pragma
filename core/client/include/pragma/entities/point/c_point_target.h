/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_POINT_TARGET_H__
#define __C_POINT_TARGET_H__
#include "pragma/clientdefinitions.h"
#include "pragma/c_enginedefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/baseentity_handle.h>
#include "pragma/entities/point/point_target.h"

namespace pragma
{
	class DLLCLIENT CPointTargetComponent final
		: public BasePointTargetComponent
	{
	public:
		CPointTargetComponent(BaseEntity &ent) : BasePointTargetComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLCLIENT CPointTarget
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif