/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/model/model.h"

using namespace pragma;

void BaseAnimatedComponent::MaintainAnimationMovement(const Vector3 &disp)
{
	CEMaintainAnimationMovement evData {disp};
	InvokeEventCallbacks(EVENT_MAINTAIN_ANIMATION_MOVEMENT, evData);
}
