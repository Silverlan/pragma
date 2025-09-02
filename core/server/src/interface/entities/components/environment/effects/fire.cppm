// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/effects/env_fire.h"

export module pragma.server.entities.components.effects.fire;

export {
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
};
