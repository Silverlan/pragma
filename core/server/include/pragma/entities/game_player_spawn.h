// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __GAME_PLAYER_SPAWN_H__
#define __GAME_PLAYER_SPAWN_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include <pragma/entities/components/base_entity_component.hpp>

namespace pragma {
	class DLLSERVER SPlayerSpawnComponent final : public BaseEntityComponent {
	  public:
		SPlayerSpawnComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER GamePlayerSpawn : public SBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
