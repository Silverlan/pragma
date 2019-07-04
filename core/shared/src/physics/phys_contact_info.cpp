#include "stdafx_shared.h"
#include "pragma/physics/phys_contact_info.hpp"

Vector3 PhysContactInfo::GetContactNormal(const Vector3 &n,int8_t controllerIndex)
{
	if(controllerIndex == 1)
		return -n;
	return n;
}

double PhysContactInfo::CalcXZDistance(const Vector3 &contactPointA,const Vector3 &contactPointB,int8_t controllerIndex)
{
	auto &localPoint = (controllerIndex == 0) ? contactPointA : contactPointB;
	return umath::pow2(localPoint.x) +umath::pow2(localPoint.z);
}

PhysContactInfo::PhysContactInfo(int8_t controllerIndex)
	: controllerIndex{controllerIndex}
{}

Vector3 PhysContactInfo::GetContactNormal() const
{
	return GetContactNormal(normalWorldOnB,controllerIndex);
}

double PhysContactInfo::CalcXZDistance() const {return CalcXZDistance(contactPointA,contactPointB,controllerIndex);}
