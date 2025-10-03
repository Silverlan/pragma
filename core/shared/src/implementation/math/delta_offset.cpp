// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "mathutil/uvec.h"

module pragma.shared;

import :math.delta_offset;

DeltaOffset::DeltaOffset(Vector3 poffset, double ptime, double pdelta) : offset(poffset), time(ptime), delta(pdelta) {}
