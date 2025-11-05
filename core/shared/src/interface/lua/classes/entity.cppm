// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.entity;

export import :entities.base_entity;

export namespace Lua {
	namespace Entity {
		DLLNETWORK void register_class(luabind::class_<pragma::ecs::BaseEntity> &classDef);
		DLLNETWORK bool IsValid(pragma::ecs::BaseEntity *ent);

		DLLNETWORK void RemoveEntityOnRemoval(pragma::ecs::BaseEntity &ent, pragma::ecs::BaseEntity &entOther);
		DLLNETWORK void RemoveEntityOnRemoval(pragma::ecs::BaseEntity &ent, pragma::ecs::BaseEntity &entOther, Bool bRemove);

		DLLNETWORK float GetAirDensity(pragma::ecs::BaseEntity &ent);
	};
};
