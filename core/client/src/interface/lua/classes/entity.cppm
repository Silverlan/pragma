// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include "pragma/entities/c_baseentity.h"
#include <pragma/networking/nwm_util.h>
#include <pragma/networking/enums.hpp>
#include <mathutil/glmutil.h>

export module pragma.client.scripting.lua.classes.entity;

export namespace Lua {
	namespace Entity {
		namespace Client {
			DLLCLIENT void register_class(luabind::class_<CBaseEntity, BaseEntity> &classDef);
			DLLCLIENT void GetEffectiveBoneTransform(lua_State *l, CBaseEntity &ent, uint32_t boneIdx);
			DLLCLIENT void SendNetEvent(lua_State *l, CBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId, const ::NetPacket &packet);
			DLLCLIENT void SendNetEvent(lua_State *l, CBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId);
		};
	};
};
