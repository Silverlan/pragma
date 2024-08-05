/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_STATIC_BVH_USER_COMPONENT_HPP__
#define __C_STATIC_BVH_USER_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_static_bvh_user_component.hpp>
#include <unordered_set>

namespace pragma {
	class DLLCLIENT CStaticBvhUserComponent final : public BaseStaticBvhUserComponent {
	  public:
		CStaticBvhUserComponent(BaseEntity &ent) : BaseStaticBvhUserComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;
	};
};

#endif
