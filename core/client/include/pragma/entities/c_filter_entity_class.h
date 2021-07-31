/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_FILTER_ENTITY_CLASS_H__
#define __C_FILTER_ENTITY_CLASS_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/basefilterentity.h>

namespace pragma
{
	class DLLCLIENT CFilterClassComponent final
		: public BaseFilterClassComponent
	{
	public:
		CFilterClassComponent(BaseEntity &ent) : BaseFilterClassComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLCLIENT CFilterEntityClass
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif