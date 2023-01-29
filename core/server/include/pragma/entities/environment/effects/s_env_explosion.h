/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_ENV_EXPLOSION_H__
#define __S_ENV_EXPLOSION_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/effects/env_explosion.h"

namespace pragma {
	class DLLSERVER SExplosionComponent final : public BaseEnvExplosionComponent {
	  public:
		SExplosionComponent(BaseEntity &ent) : BaseEnvExplosionComponent(ent) {}
		virtual void Explode() override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};
class DLLSERVER EnvExplosion : public SBaseEntity {
  protected:
  public:
	virtual void Initialize() override;
};

#endif
