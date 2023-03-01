/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PHYS_LIQUID_HPP__
#define __PHYS_LIQUID_HPP__

#include "pragma/networkdefinitions.h"

const auto PHYS_LIQUID_DEFAULT_DENSITY = 1000.0;
const auto PHYS_LIQUID_DEFAULT_LINEAR_DRAG_COEFFICIENT = 2.0;
const auto PHYS_LIQUID_DEFAULT_TORQUE_DRAG_COEFFICIENT = 6.0;
const auto PHYS_LIQUID_DEFAULT_STIFFNESS = 0.0001f;
const auto PHYS_LIQUID_DEFAULT_PROPAGATION = 0.02f;

struct DLLNETWORK PhysLiquid {
	double density = PHYS_LIQUID_DEFAULT_DENSITY;
	double linearDragCoefficient = PHYS_LIQUID_DEFAULT_LINEAR_DRAG_COEFFICIENT;
	double torqueDragCoefficient = PHYS_LIQUID_DEFAULT_TORQUE_DRAG_COEFFICIENT;
	float stiffness = PHYS_LIQUID_DEFAULT_STIFFNESS;
	float propagation = PHYS_LIQUID_DEFAULT_PROPAGATION;
};

#endif
