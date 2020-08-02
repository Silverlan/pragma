/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/lsweep.h"
#include <mathutil/glmutil.h>
#include "pragma/lua/ldefinitions.h"
#include "pragma/lua/classes/ldef_vector.h"
#include <pragma/math/intersection.h>
#include "luasystem.h"
void Lua::sweep::AABBWithAABB(const Vector3 &originA,const Vector3 &endA,const Vector3 &extentsA,const Vector3 &originB,const Vector3 &endB,const Vector3 &extentsB,bool &outResult,float &outTEntry,float &outTExit,Vector3 &outNormal)
{
	outResult = Sweep::AABBWithAABB(originA,endA,extentsA,originB,endB,extentsB,&outTEntry,&outTExit,&outNormal);
}

void Lua::sweep::AABBWithPlane(const Vector3 &ext,const Vector3 &origin,const Vector3 &dir,const Vector3 &n,float d,bool &outResult,float &outT)
{
	outResult = Sweep::AABBWithPlane(origin,dir,ext,n,d,&outT);
}