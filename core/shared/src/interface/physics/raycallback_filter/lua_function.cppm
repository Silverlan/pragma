// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/physics/raycallback/physraycallbackfilter.hpp"
#include "pragma/lua/luafunction.h"

export module pragma.shared:physics.raycallback.filter_lua_function;

export class DLLNETWORK BasePhysRayCallbackFilterLuaFunction : public BasePhysRayCallbackFilter {
  protected:
	LuaFunction m_filter;
  public:
	BasePhysRayCallbackFilterLuaFunction(const LuaFunction &filter, FTRACE flags, CollisionMask group, CollisionMask mask);
	virtual bool ShouldPass(BaseEntity *ent, PhysObj *phys, pragma::physics::ICollisionObject *obj) override;
};
