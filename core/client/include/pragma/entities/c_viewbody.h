/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_VIEWBODY_H__
#define __C_VIEWBODY_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"

namespace pragma
{
	class DLLCLIENT CViewBodyComponent final
		: public BaseEntityComponent
	{
	public:
		CViewBodyComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLCLIENT CViewBody
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif