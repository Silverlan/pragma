// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:physics.touch;

export import :entities.base_entity_handle;

export class pragma::ecs::BaseEntity;
export struct DLLNETWORK PhysTouch {
	PhysTouch(pragma::ecs::BaseEntity *ent, CallbackHandle onRemove);
	~PhysTouch();
	EntityHandle entity;
	CallbackHandle onRemoveCallback;
};
