// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <sharedutils/functioncallback.h>
#include "pragma/entities/baseentity_handle.h"

export module pragma.shared:physics.phys_touch;

export struct DLLNETWORK PhysTouch {
	PhysTouch(BaseEntity *ent, CallbackHandle onRemove);
	~PhysTouch();
	EntityHandle entity;
	CallbackHandle onRemoveCallback;
};
