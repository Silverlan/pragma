// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"



export module pragma.server.entities.components.bot;

import pragma.server.entities.base;

export {
	namespace pragma {
		class DLLSERVER SBotComponent final : public BaseBotComponent {
		public:
			SBotComponent(BaseEntity &ent) : BaseBotComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER Bot : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
