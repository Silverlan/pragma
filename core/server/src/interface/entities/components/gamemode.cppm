// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.gamemode;

import :entities;

export {
	namespace pragma {
		class DLLSERVER SGamemodeComponent final : public BaseGamemodeComponent {
		  public:
			SGamemodeComponent(ecs::BaseEntity &ent) : BaseGamemodeComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER SGamemode : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
