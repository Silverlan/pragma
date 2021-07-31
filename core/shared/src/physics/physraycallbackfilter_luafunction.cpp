/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/physics/raycallback/physraycallbackfilter_luafunction.hpp"
#include "pragma/lua/luafunction_call.h"

BasePhysRayCallbackFilterLuaFunction::BasePhysRayCallbackFilterLuaFunction(const LuaFunction &filter,FTRACE flags,CollisionMask group,CollisionMask mask)
	: BasePhysRayCallbackFilter(flags,group,mask),m_filter(filter)
{}
bool BasePhysRayCallbackFilterLuaFunction::ShouldPass(BaseEntity *ent,PhysObj *phys,pragma::physics::ICollisionObject *col)
{
	if(BasePhysRayCallbackFilter::ShouldPass(ent,phys,col) == false)
		return false;
	Bool bShouldPass = false;
	if(m_filter.Call<bool,luabind::object,PhysObjHandle>(&bShouldPass,ent->GetLuaObject(),phys->GetHandle()) == true)
		return TranslateFilterValue(bShouldPass);
	return false;
}
