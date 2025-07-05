// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/math/deltatransform.h"

DeltaTransform::DeltaTransform(Vector3 poffset, Quat porientation, double ptime, double pdelta) : offset(poffset), orientation(porientation), time(ptime), delta(pdelta) {}
