// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"


export module pragma.client:scripting.lua.classes.entity;

export import :entities.base_entity;

export namespace Lua {
	namespace Entity {
		namespace Client {
			DLLCLIENT void register_class(luabind::class_<CBaseEntity, pragma::ecs::BaseEntity> &classDef);
			DLLCLIENT void GetEffectiveBoneTransform(lua_State *l, CBaseEntity &ent, uint32_t boneIdx);
			DLLCLIENT void SendNetEvent(lua_State *l, CBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId, const ::NetPacket &packet);
			DLLCLIENT void SendNetEvent(lua_State *l, CBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId);
		};
	};
};
