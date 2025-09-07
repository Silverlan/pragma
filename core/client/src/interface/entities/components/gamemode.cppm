// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/components/base_gamemode_component.hpp>

export module pragma.client.entities.components.gamemode;

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
