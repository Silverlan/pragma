// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __DELTAOFFSET_H__
#define __DELTAOFFSET_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>

struct DLLNETWORK DeltaOffset {
	DeltaOffset(Vector3 poffset, double ptime, double pdelta = 0.f);
	double time = 0.0;
	double delta = 0.0;
	Vector3 offset = {};
};

#endif
