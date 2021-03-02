/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __BASEWATEROBJECT_HPP__
#define __BASEWATEROBJECT_HPP__

#include "pragma/networkdefinitions.h"
#include <mathutil/plane.hpp>

class DLLNETWORK BaseWaterObject
{
public:
	void GetWaterPlane(Vector3 &n,double &d) const;
	void GetWaterPlaneWs(Vector3 &n,double &d) const;
	bool IsPointBelowWaterPlane(const Vector3 &p);
	virtual const Vector3 &GetPosition() const=0;
	virtual const Quat &GetOrientation() const=0;
protected:
	umath::Plane m_waterPlane = {{0.f,1.f,0.f},1.f};
};

#endif
