// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "mathutil/umath.h"

module pragma.shared;

import :physics.raycallback.filter_lua_function;

BasePhysRayCallbackFilterLuaFunction::BasePhysRayCallbackFilterLuaFunction(const LuaFunction &filter, FTRACE flags, CollisionMask group, CollisionMask mask) : BasePhysRayCallbackFilter(flags, group, mask), m_filter(filter) {}
bool BasePhysRayCallbackFilterLuaFunction::ShouldPass(BaseEntity *ent, PhysObj *phys, pragma::physics::ICollisionObject *col)
{
	if(BasePhysRayCallbackFilter::ShouldPass(ent, phys, col) == false)
		return false;
	Bool bShouldPass = false;
	if(m_filter.Call<bool, luabind::object, PhysObjHandle>(&bShouldPass, ent->GetLuaObject(), phys->GetHandle()) == true)
		return TranslateFilterValue(bShouldPass);
	return false;
}
