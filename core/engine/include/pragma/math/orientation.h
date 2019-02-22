#ifndef __ORIENTATION_H__
#define __ORIENTATION_H__
#include "pragma/definitions.h"
#include <iostream>
#include <vector>
#include <mathutil/glmutil.h>
#include "pragma/math/angle/wvangle.h"

struct DLLENGINE AngleOrientation
{
	AngleOrientation() {AngleOrientation(Vector3(0,0,0),EulerAngles(0,0,0));}
	AngleOrientation(Vector3 pos,EulerAngles ang)
	{
		this->pos = pos;
		this->ang = ang;
	}
	Vector3 pos;
	EulerAngles ang;
};

struct DLLENGINE Orientation
{
	Orientation() {Orientation(Vector3(0,0,0),uquat::identity());}
	Orientation(Vector3 pos,Quat rot)
	{
		this->pos = pos;
		this->rot = rot;
	}
	Vector3 pos = {};
	Quat rot = uquat::identity();
};
#endif