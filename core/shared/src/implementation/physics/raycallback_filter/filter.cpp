// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


module pragma.shared;

import :physics.raycallback.filter;

CollisionMask get_collision_group(CollisionMask group, CollisionMask mask)
{
	if(group == CollisionMask::Default)
		return mask;
	return group;
}

BasePhysRayCallbackFilter::BasePhysRayCallbackFilter(FTRACE flags, CollisionMask group, CollisionMask mask) : m_flags(flags)
{
	if((UInt32(flags) & UInt32(RayCastFlags::InvertFilter)) != 0 && mask == CollisionMask::All)
		mask = CollisionMask::None;
	m_filterGroup = get_collision_group(group, mask);
	m_filterMask = mask;
}

void BasePhysRayCallbackFilter::SetUserData(void *userData) const { m_userData = userData; }
void *BasePhysRayCallbackFilter::GetUserData() const { return m_userData; }

bool BasePhysRayCallbackFilter::ShouldPass(BaseEntity *, PhysObj *phys, pragma::physics::ICollisionObject *physCol)
{
	if(phys == nullptr)
		return true;
	if((umath::to_integral(m_flags) & umath::to_integral(RayCastFlags::IgnoreDynamic)) != 0 && !phys->IsStatic() || (umath::to_integral(m_flags) & umath::to_integral(RayCastFlags::IgnoreStatic)) != 0 && phys->IsStatic())
		return false;
	if(physCol == nullptr)
		return true;
	if((umath::to_integral(m_filterMask) & umath::to_integral(physCol->GetCollisionFilterGroup())) == 0)
		return TranslateFilterValue(false);
	return true;
}

bool BasePhysRayCallbackFilter::TranslateFilterValue(bool b) const { return ((UInt32(m_flags) & UInt32(RayCastFlags::InvertFilter)) == 0) ? b : !b; }
