// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/math/angle/wvquaternion.h"

DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const Quat &q)
{
	os << q.w << " " << q.x << " " << q.y << " " << q.z;
	return os;
}
