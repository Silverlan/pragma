// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
