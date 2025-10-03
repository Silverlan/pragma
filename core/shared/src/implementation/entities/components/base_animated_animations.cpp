// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "mathutil/uvec.h"

module pragma.shared;

import :entities.components.base_animated;

using namespace pragma;

void BaseAnimatedComponent::MaintainAnimationMovement(const Vector3 &disp)
{
	CEMaintainAnimationMovement evData {disp};
	InvokeEventCallbacks(EVENT_MAINTAIN_ANIMATION_MOVEMENT, evData);
}
