#include "stdafx_shared.h"
#include "pragma/basewaterobject.hpp"

void BaseWaterObject::GetWaterPlane(Vector3 &n,double &d) const
{
	n = m_waterPlane.GetNormal();
	d = m_waterPlane.GetDistance();
	// d += m_kvMaxWaveHeight;
}
void BaseWaterObject::GetWaterPlaneWs(Vector3 &n,double &d) const
{
	GetWaterPlane(n,d);
	uvec::rotate(&n,GetOrientation());
	d = -uvec::dot(n,n *static_cast<float>(d) -GetPosition());
}
bool BaseWaterObject::IsPointBelowWaterPlane(const Vector3 &p)
{
	Vector3 n;
	double d;
	GetWaterPlaneWs(n,d);
	auto pProj = uvec::project_to_plane(p,n,d);
	auto dot = uvec::dot(n,p -pProj);
	return (dot < 0.f) ? true : false;
}
