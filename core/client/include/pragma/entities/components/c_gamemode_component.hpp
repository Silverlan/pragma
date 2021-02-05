/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_GAMEMODE_COMPONENT_HPP__
#define __C_GAMEMODE_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_gamemode_component.hpp>

namespace pragma
{
	class DLLCLIENT CGamemodeComponent final
		: public BaseGamemodeComponent
	{
	public:
		CGamemodeComponent(BaseEntity &ent) : BaseGamemodeComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLCLIENT CGamemode
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
