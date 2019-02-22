#include "stdafx_shared.h"
#include "pragma/physics/phys_contact_info.hpp"

Vector3 PhysContactInfo::GetContactNormal(const Vector3 &n,int8_t controllerIndex)
{
	if(controllerIndex == 1)
		return -n;
	return n;
}

double PhysContactInfo::CalcXZDistance(const btManifoldPoint &contactPoint,int8_t controllerIndex)
{
	auto &localPoint = (controllerIndex == 0) ? contactPoint.m_localPointA : contactPoint.m_localPointB;
	return umath::pow2(localPoint.x()) +umath::pow2(localPoint.z());
}

PhysContactInfo::PhysContactInfo(const btManifoldPoint &contactPoint,int8_t controllerIndex)
	: contactPoint{contactPoint},controllerIndex{controllerIndex}
{}

Vector3 PhysContactInfo::GetContactNormal() const
{
	return GetContactNormal(uvec::create(contactPoint.m_normalWorldOnB),controllerIndex);
}

double PhysContactInfo::CalcXZDistance() const {return CalcXZDistance(contactPoint,controllerIndex);}
