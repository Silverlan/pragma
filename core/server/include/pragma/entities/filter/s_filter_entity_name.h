// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_FILTER_ENTITY_NAME_H__
#define __S_FILTER_ENTITY_NAME_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include <pragma/entities/basefilterentity.h>

namespace pragma {
	class DLLSERVER SFilterNameComponent final : public BaseFilterNameComponent {
	  public:
		SFilterNameComponent(BaseEntity &ent) : BaseFilterNameComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER FilterEntityName : public SBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
