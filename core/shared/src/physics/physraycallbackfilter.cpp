/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/physics/raycallback/physraycallbackfilter.hpp"
#include "pragma/physics/raytraces.h"
#include "pragma/entities/components/base_physics_component.hpp"

CollisionMask get_collision_group(CollisionMask group,CollisionMask mask)
{
	if(group == CollisionMask::Default)
		return mask;
	return group;
}

BasePhysRayCallbackFilter::BasePhysRayCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask)
	: m_flags(flags)
{
	if((UInt32(flags) &UInt32(RayCastFlags::InvertFilter)) != 0 && mask == CollisionMask::All)
		mask = CollisionMask::None;
	m_filterGroup = get_collision_group(group,mask);
	m_filterMask = mask;
}

void BasePhysRayCallbackFilter::SetUserData(void *userData) const {m_userData = userData;}
void *BasePhysRayCallbackFilter::GetUserData() const {return m_userData;}

bool BasePhysRayCallbackFilter::ShouldPass(BaseEntity*,PhysObj *phys,pragma::physics::ICollisionObject *physCol)
{
	if(phys == nullptr)
		return true;
	if(
		(umath::to_integral(m_flags) &umath::to_integral(RayCastFlags::IgnoreDynamic)) != 0 && !phys->IsStatic() ||
		(umath::to_integral(m_flags) &umath::to_integral(RayCastFlags::IgnoreStatic)) != 0 && phys->IsStatic()
	)
		return false;
	if(physCol == nullptr)
		return true;
	if((umath::to_integral(m_filterMask) &umath::to_integral(physCol->GetCollisionFilterGroup())) == 0)
		return TranslateFilterValue(false);
	return true;
}

bool BasePhysRayCallbackFilter::TranslateFilterValue(bool b) const
{
	return ((UInt32(m_flags) &UInt32(RayCastFlags::InvertFilter)) == 0) ? b : !b;
}
