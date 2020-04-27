/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __PLANE_H__
#define __PLANE_H__
#include "pragma/networkdefinitions.h"
#include <string>

#include <vector>
#include <iostream>
#include <mathutil/eulerangles.h>
#include <mathutil/glmutil.h>

#include <pragma/math/vector/wvvector3.h>

class DLLNETWORK Plane
{
public:
	Plane(const Vector3 &a,const Vector3 &b,const Vector3 &c);
	Plane(Vector3 n,const Vector3 &pos);
	Plane(Vector3 n,double d);
	Plane();
private:
	Vector3 m_normal;
	Vector3 m_pos;
	Vector3 m_posCenter;
	double m_distance;
	void Initialize(Vector3 n,double d);
public:
	static bool GetPlaneIntersection(Vector3 *intersect,const Vector3 &na,const Vector3 &nb,const Vector3 &nc,double da,double db,double dc);
	const Vector3 &GetNormal() const;
	const Vector3 &GetPos() const;
	const Vector3 &GetCenterPos() const;
	Vector3 &GetNormal();
	Vector3 &GetPos();
	Vector3 &GetCenterPos();
	double GetDistance() const;
	float GetDistance(const Vector3 &pos) const;
	void SetDistance(double d);
	void SetNormal(const Vector3 &n);
	void MoveToPos(const Vector3 &pos);
	void Rotate(const EulerAngles &ang);
};
#endif