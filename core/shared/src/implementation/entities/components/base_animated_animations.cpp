// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_animated;

using namespace pragma;

void BaseAnimatedComponent::MaintainAnimationMovement(const Vector3 &disp)
{
	CEMaintainAnimationMovement evData {disp};
	InvokeEventCallbacks(baseAnimatedComponent::EVENT_MAINTAIN_ANIMATION_MOVEMENT, evData);
}
