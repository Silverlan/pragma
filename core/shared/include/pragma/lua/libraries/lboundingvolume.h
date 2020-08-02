/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LBOUNDINGVOLUME_H__
#define __LBOUNDINGVOLUME_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace Lua::boundingvolume
{
	DLLNETWORK void GetRotatedAABB(const Vector3 &min,const Vector3 &max,const Mat4 &rot,Vector3 &rmin,Vector3 &rmax);
};

#endif
