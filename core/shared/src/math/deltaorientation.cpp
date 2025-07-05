// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/math/deltaorientation.h"

DeltaOrientation::DeltaOrientation(Quat porientation, double ptime, double pdelta) : orientation(porientation), time(ptime), delta(pdelta) {}
