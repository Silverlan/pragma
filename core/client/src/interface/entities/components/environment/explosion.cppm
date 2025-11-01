// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"



export module pragma.client:entities.components.effects.explosion;

export import :entities.base_entity;

export namespace pragma {
	class DLLCLIENT CExplosionComponent final : public BaseEnvExplosionComponent {
	  public:
		CExplosionComponent(pragma::ecs::BaseEntity &ent) : BaseEnvExplosionComponent(ent) {}
		virtual void Initialize() override;
		virtual void Explode() override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

export class DLLCLIENT CEnvExplosion : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
