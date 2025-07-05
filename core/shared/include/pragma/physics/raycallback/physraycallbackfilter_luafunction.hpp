// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PHYS_RAYCALLBACKFILTER_LUAFUNCTION_HPP__
#define __PHYS_RAYCALLBACKFILTER_LUAFUNCTION_HPP__

#include "pragma/physics/raycallback/physraycallbackfilter.hpp"
#include "pragma/lua/luafunction.h"

class DLLNETWORK BasePhysRayCallbackFilterLuaFunction : public BasePhysRayCallbackFilter {
  protected:
	LuaFunction m_filter;
  public:
	BasePhysRayCallbackFilterLuaFunction(const LuaFunction &filter, FTRACE flags, CollisionMask group, CollisionMask mask);
	virtual bool ShouldPass(BaseEntity *ent, PhysObj *phys, pragma::physics::ICollisionObject *obj) override;
};

#endif
