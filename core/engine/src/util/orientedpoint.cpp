#include "stdafx_engine.h"
#include "pragma/util/orientedpoint.h"

OrientedPoint::OrientedPoint()
{}
OrientedPoint::OrientedPoint(Vector3 &position)
	: pos(position)
{}
OrientedPoint::OrientedPoint(Quat &orientation)
	: rot(orientation)
{}
OrientedPoint::OrientedPoint(Vector3 &position,Quat &orientation)
	: pos(position),rot(orientation)
{}