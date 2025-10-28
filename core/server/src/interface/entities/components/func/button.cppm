// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"



export module pragma.server.entities.components.func.button;

import pragma.server.entities;

export {
	namespace pragma {
		class DLLSERVER SButtonComponent final : public BaseFuncButtonComponent {
		public:
			SButtonComponent(pragma::ecs::BaseEntity &ent) : BaseFuncButtonComponent(ent) {}
			virtual void Initialize() override;
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER FuncButton : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
