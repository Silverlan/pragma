// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:physics.touch;

export import :entities.base_entity_handle;

export namespace pragma::ecs {
	class BaseEntity;
}
export namespace pragma::physics {
	struct DLLNETWORK PhysTouch {
		PhysTouch(ecs::BaseEntity *ent, CallbackHandle onRemove);
		~PhysTouch();
		EntityHandle entity;
		CallbackHandle onRemoveCallback;
	};
}
