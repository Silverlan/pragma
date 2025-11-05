// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.server:entities.components.func.portal;

import :entities;

export {
	namespace pragma {
		class DLLSERVER SFuncPortalComponent final : public BaseFuncPortalComponent {
		  public:
			SFuncPortalComponent(pragma::ecs::BaseEntity &ent) : BaseFuncPortalComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER FuncPortal : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
