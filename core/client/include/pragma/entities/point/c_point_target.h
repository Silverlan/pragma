// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_POINT_TARGET_H__
#define __C_POINT_TARGET_H__
#include "pragma/clientdefinitions.h"
#include "pragma/c_enginedefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/baseentity_handle.h>
#include "pragma/entities/point/point_target.h"

namespace pragma {
	class DLLCLIENT CPointTargetComponent final : public BasePointTargetComponent {
	  public:
		CPointTargetComponent(BaseEntity &ent) : BasePointTargetComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLCLIENT CPointTarget : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
