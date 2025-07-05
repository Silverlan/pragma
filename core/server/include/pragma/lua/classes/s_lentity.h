// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_LENTITY_H__
#define __S_LENTITY_H__
#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include "pragma/lua/classes/ldef_entity.h"

namespace pragma::networking {
	class TargetRecipientFilter;
};
namespace Lua {
	namespace Entity {
		namespace Server {
			DLLSERVER void register_class(luabind::class_<SBaseEntity, BaseEntity> &classDef);
			DLLSERVER void SendNetEvent(lua_State *l, SBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId, ::NetPacket packet, pragma::networking::TargetRecipientFilter &rp);
			DLLSERVER void SendNetEvent(lua_State *l, SBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId, ::NetPacket packet);
			DLLSERVER void SendNetEvent(lua_State *l, SBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId, pragma::networking::TargetRecipientFilter &rp);
			DLLSERVER void SendNetEvent(lua_State *l, SBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId);
		};
	};
};

#endif
