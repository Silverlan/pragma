// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <string>

module pragma.shared;

import :math.delta_transform;

DeltaTransform::DeltaTransform(Vector3 poffset, Quat porientation, double ptime, double pdelta) : offset(poffset), orientation(porientation), time(ptime), delta(pdelta) {}
