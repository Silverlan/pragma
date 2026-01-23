// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :math.delta_orientation;

pragma::math::DeltaOrientation::DeltaOrientation(Quat porientation, double ptime, double pdelta) : orientation(porientation), time(ptime), delta(pdelta) {}
