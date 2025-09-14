// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/basefilterentity.h>

export module pragma.client.entities.components.filter_entity_name;

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
