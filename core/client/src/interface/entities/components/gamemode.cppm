// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"



export module pragma.client:entities.components.gamemode;

import :entities.base_entity;

export namespace pragma {
	class DLLCLIENT CGamemodeComponent final : public BaseGamemodeComponent {
	  public:
		CGamemodeComponent(BaseEntity &ent) : BaseGamemodeComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

export class DLLCLIENT CGamemode : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
