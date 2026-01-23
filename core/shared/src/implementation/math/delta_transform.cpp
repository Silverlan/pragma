// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :math.delta_transform;

pragma::math::DeltaTransform::DeltaTransform(Vector3 poffset, Quat porientation, double ptime, double pdelta) : offset(poffset), orientation(porientation), time(ptime), delta(pdelta) {}
