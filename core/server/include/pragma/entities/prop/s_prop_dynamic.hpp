/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_PROP_DYNAMIC_HPP__
#define __S_PROP_DYNAMIC_HPP__

#include "pragma/serverdefinitions.h"
#include <pragma/entities/prop/prop_dynamic.hpp>

namespace pragma
{
	class DLLSERVER SPropDynamicComponent final
		: public BasePropDynamicComponent
	{
	public:
		SPropDynamicComponent(BaseEntity &ent) : BasePropDynamicComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER PropDynamic
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif