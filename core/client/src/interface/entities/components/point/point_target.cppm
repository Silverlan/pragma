// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"



export module pragma.client:entities.components.point_target;

import :entities.base_entity;

export {
	namespace pragma {
		class DLLCLIENT CPointTargetComponent final : public BasePointTargetComponent {
		public:
			CPointTargetComponent(BaseEntity &ent) : BasePointTargetComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLCLIENT CPointTarget : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
