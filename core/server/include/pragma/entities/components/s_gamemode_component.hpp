/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_GAMEMODE_COMPONENT_HPP__
#define __S_GAMEMODE_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include <pragma/entities/components/base_gamemode_component.hpp>

namespace pragma
{
	class DLLSERVER SGamemodeComponent final
		: public BaseGamemodeComponent
	{
	public:
		SGamemodeComponent(BaseEntity &ent) : BaseGamemodeComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER SGamemode
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
