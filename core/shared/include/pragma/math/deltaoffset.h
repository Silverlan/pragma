/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __DELTAOFFSET_H__
#define __DELTAOFFSET_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>

struct DLLNETWORK DeltaOffset
{
	DeltaOffset(Vector3 poffset,double ptime,double pdelta=0.f);
	double time = 0.0;
	double delta = 0.0;
	Vector3 offset = {};
};

#endif