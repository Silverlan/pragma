// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/luaapi.h"
#include <mathutil/glmutil.h>

export module pragma.shared:scripting.lua.classes.entity;

export import :entities.base_entity;

export namespace Lua {
	namespace Entity {
		DLLNETWORK void register_class(luabind::class_<BaseEntity> &classDef);
		DLLNETWORK bool IsValid(BaseEntity *ent);

		DLLNETWORK void RemoveEntityOnRemoval(BaseEntity &ent, BaseEntity &entOther);
		DLLNETWORK void RemoveEntityOnRemoval(BaseEntity &ent, BaseEntity &entOther, Bool bRemove);

		DLLNETWORK float GetAirDensity(BaseEntity &ent);
	};
};
