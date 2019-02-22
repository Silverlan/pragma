#ifndef __BASEWATEROBJECT_HPP__
#define __BASEWATEROBJECT_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/math/plane.h"

class DLLNETWORK BaseWaterObject
{
public:
	void GetWaterPlane(Vector3 &n,double &d) const;
	void GetWaterPlaneWs(Vector3 &n,double &d) const;
	bool IsPointBelowWaterPlane(const Vector3 &p);
	virtual const Vector3 &GetPosition() const=0;
	virtual const Quat &GetOrientation() const=0;
protected:
	Plane m_waterPlane = {{0.f,1.f,0.f},1.f};
};

#endif
