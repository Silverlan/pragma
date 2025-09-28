// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.client:entities.components.effects.explosion;

import :entities.base_entity;

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
