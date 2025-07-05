// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_POINT_PATH_NODE_H__
#define __S_POINT_PATH_NODE_H__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/components/basepointpathnode.h"
#include "pragma/entities/components/s_entity_component.hpp"

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

#endif
