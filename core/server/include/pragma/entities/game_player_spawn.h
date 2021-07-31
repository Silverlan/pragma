/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __GAME_PLAYER_SPAWN_H__
#define __GAME_PLAYER_SPAWN_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include <pragma/entities/components/base_entity_component.hpp>

namespace pragma
{
	class DLLSERVER SPlayerSpawnComponent final
		: public BaseEntityComponent
	{
	public:
		SPlayerSpawnComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER GamePlayerSpawn
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif