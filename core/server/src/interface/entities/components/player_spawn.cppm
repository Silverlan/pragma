// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.player_spawn;

import :entities.base;

export {
	namespace pragma {
		class DLLSERVER SPlayerSpawnComponent final : public BaseEntityComponent {
		  public:
			SPlayerSpawnComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER GamePlayerSpawn : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
