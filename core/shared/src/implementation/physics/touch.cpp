// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;



module pragma.shared;

import :physics.touch;

PhysTouch::PhysTouch(pragma::ecs::BaseEntity *ent, CallbackHandle onRemove) : entity((ent != nullptr) ? ent->GetHandle() : EntityHandle()), onRemoveCallback(onRemove) {}
PhysTouch::~PhysTouch()
{
	if(onRemoveCallback.IsValid())
		onRemoveCallback.Remove();
}
