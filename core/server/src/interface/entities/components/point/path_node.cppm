// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/components/basepointpathnode.h"
#include "pragma/entities/components/s_entity_component.hpp"

export module pragma.server.entities.components.point.path_node;

export {
	namespace pragma {
		class DLLSERVER SPathNodeComponent final : public BasePointPathNodeComponent {
		public:
			SPathNodeComponent(BaseEntity &ent) : BasePointPathNodeComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER PointPathNode : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
