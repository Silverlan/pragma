// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.effects.explosion;

import :entities;

export {
	namespace pragma {
		class DLLSERVER SExplosionComponent final : public BaseEnvExplosionComponent {
		  public:
			SExplosionComponent(ecs::BaseEntity &ent) : BaseEnvExplosionComponent(ent) {}
			virtual void Explode() override;
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};
	class DLLSERVER EnvExplosion : public SBaseEntity {
	  protected:
	  public:
		virtual void Initialize() override;
	};
};
