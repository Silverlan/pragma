// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"


export module pragma.server.entities.components.player_spawn;

import pragma.server.entities.base;

export {
	namespace pragma {
		class DLLSERVER SPlayerSpawnComponent final : public BaseEntityComponent {
		public:
			SPlayerSpawnComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER GamePlayerSpawn : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
