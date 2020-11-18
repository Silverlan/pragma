/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __WORLD_H__
#define __WORLD_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include <pragma/entities/baseworld.h>

namespace pragma
{
	class DLLSERVER SWorldComponent final
		: public BaseWorldComponent
	{
	public:
		SWorldComponent(BaseEntity &ent) : BaseWorldComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class SPolyMesh;
class DLLSERVER World
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif