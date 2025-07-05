// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_FILTER_ENTITY_CLASS_H__
#define __S_FILTER_ENTITY_CLASS_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include <pragma/entities/basefilterentity.h>

namespace pragma {
	class DLLSERVER SFilterClassComponent final : public BaseFilterClassComponent {
	  public:
		SFilterClassComponent(BaseEntity &ent) : BaseFilterClassComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER FilterEntityClass : public SBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
