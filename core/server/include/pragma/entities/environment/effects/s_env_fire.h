// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_ENV_FIRE_H__
#define __S_ENV_FIRE_H__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/effects/env_fire.h"

namespace pragma {
	class DLLSERVER SFireComponent final : public BaseEnvFireComponent {
	  public:
		SFireComponent(BaseEntity &ent) : BaseEnvFireComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER EnvFire : public SBaseEntity {
  protected:
  public:
	virtual void Initialize() override;
};

#endif
