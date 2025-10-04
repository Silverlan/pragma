// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "mathutil/uquat.h"

module pragma.shared;

import :math.delta_orientation;

DeltaOrientation::DeltaOrientation(Quat porientation, double ptime, double pdelta) : orientation(porientation), time(ptime), delta(pdelta) {}
