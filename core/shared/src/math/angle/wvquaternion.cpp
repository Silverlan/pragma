/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/math/angle/wvquaternion.h"

DLLNETWORK Con::c_cout & operator<<(Con::c_cout &os,const Quat &q)
{
	os<<q.w<<" "<<q.x<<" "<<q.y<<" "<<q.z;
	return os;
}