// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.bot;

import :entities.base;

export {
	namespace pragma {
		class DLLSERVER SBotComponent final : public BaseBotComponent {
		  public:
			SBotComponent(ecs::BaseEntity &ent) : BaseBotComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER Bot : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
