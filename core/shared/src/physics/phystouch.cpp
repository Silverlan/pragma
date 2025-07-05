// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/entities/baseentity.h"
#include "pragma/physics/phystouch.h"

PhysTouch::PhysTouch(BaseEntity *ent, CallbackHandle onRemove) : entity((ent != nullptr) ? ent->GetHandle() : EntityHandle()), onRemoveCallback(onRemove) {}
PhysTouch::~PhysTouch()
{
	if(onRemoveCallback.IsValid())
		onRemoveCallback.Remove();
}
