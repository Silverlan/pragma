// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.server:entities.components.effects.explosion;

import :entities;

export {
	namespace pragma {
		class DLLSERVER SExplosionComponent final : public BaseEnvExplosionComponent {
		  public:
			SExplosionComponent(pragma::ecs::BaseEntity &ent) : BaseEnvExplosionComponent(ent) {}
			virtual void Explode() override;
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};
	class DLLSERVER EnvExplosion : public SBaseEntity {
	  protected:
	  public:
		virtual void Initialize() override;
	};
};
