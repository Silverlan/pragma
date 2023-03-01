/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

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
