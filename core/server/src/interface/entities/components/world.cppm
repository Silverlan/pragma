// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/luaapi.h"



export module pragma.server.entities.components.world;

import pragma.server.entities;

export {
	namespace pragma {
		class DLLSERVER SWorldComponent final : public BaseWorldComponent {
		public:
			SWorldComponent(BaseEntity &ent) : BaseWorldComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER World : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
