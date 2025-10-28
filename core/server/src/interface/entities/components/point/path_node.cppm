// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"



export module pragma.server.entities.components.point.path_node;

import pragma.server.entities;
import pragma.server.entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SPathNodeComponent final : public BasePointPathNodeComponent {
		public:
			SPathNodeComponent(pragma::ecs::BaseEntity &ent) : BasePointPathNodeComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER PointPathNode : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
