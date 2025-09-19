// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/effects/env_explosion.h"

export module pragma.client.entities.components.effects.explosion;

export namespace pragma {
	class DLLCLIENT CExplosionComponent final : public BaseEnvExplosionComponent {
	  public:
		CExplosionComponent(BaseEntity &ent) : BaseEnvExplosionComponent(ent) {}
		virtual void Initialize() override;
		virtual void Explode() override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

export class DLLCLIENT CEnvExplosion : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
