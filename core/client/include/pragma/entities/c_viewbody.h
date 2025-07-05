// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_VIEWBODY_H__
#define __C_VIEWBODY_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"

namespace pragma {
	class DLLCLIENT CViewBodyComponent final : public BaseEntityComponent {
	  public:
		CViewBodyComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLCLIENT CViewBody : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
