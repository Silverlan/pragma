// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/math/angle/wvangle.h"
#include "pragma/console/conout.h"

DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const EulerAngles &ang)
{
	os << ang.p << " " << ang.y << " " << ang.r;
	return os;
}
