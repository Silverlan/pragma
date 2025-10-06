// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/luaapi.h"



export module pragma.server.entities.components.point.target;

import pragma.server.entities;

export {
	namespace pragma {
		class DLLSERVER SPointTargetComponent final : public BasePointTargetComponent {
		public:
			SPointTargetComponent(BaseEntity &ent) : BasePointTargetComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER PointTarget : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
