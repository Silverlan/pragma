/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/math/deltatransform.h"

DeltaTransform::DeltaTransform(Vector3 poffset,Quat porientation,double ptime,double pdelta)
	: offset(poffset),orientation(porientation),time(ptime),delta(pdelta)
{}