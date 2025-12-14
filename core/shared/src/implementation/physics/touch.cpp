// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :physics.touch;

pragma::physics::PhysTouch::PhysTouch(ecs::BaseEntity *ent, CallbackHandle onRemove) : entity((ent != nullptr) ? ent->GetHandle() : EntityHandle()), onRemoveCallback(onRemove) {}
pragma::physics::PhysTouch::~PhysTouch()
{
	if(onRemoveCallback.IsValid())
		onRemoveCallback.Remove();
}
