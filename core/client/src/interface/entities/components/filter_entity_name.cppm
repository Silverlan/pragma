// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"



export module pragma.client:entities.components.filter_entity_name;

import :entities.base_entity;

export namespace pragma {
	class DLLCLIENT CFilterNameComponent final : public BaseFilterNameComponent {
	  public:
		CFilterNameComponent(BaseEntity &ent) : BaseFilterNameComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

export class DLLCLIENT CFilterEntityName : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
