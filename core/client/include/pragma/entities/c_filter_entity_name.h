// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_FILTER_ENTITY_NAME_H__
#define __C_FILTER_ENTITY_NAME_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/basefilterentity.h>

namespace pragma {
	class DLLCLIENT CFilterNameComponent final : public BaseFilterNameComponent {
	  public:
		CFilterNameComponent(BaseEntity &ent) : BaseFilterNameComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLCLIENT CFilterEntityName : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
