// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"



export module pragma.server.entities.components.effects.fire;

import pragma.server.entities;

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
