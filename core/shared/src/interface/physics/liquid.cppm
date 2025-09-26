// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:physics.liquid;

export {
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
};
