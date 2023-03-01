/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LENTITY_H__
#define __C_LENTITY_H__
#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include <mathutil/glmutil.h>
class NetPacket;
namespace nwm {
	enum class Protocol : uint32_t;
};
namespace Lua {
	namespace Entity {
		namespace Client {
			DLLCLIENT void register_class(luabind::class_<CBaseEntity, BaseEntity> &classDef);
			DLLCLIENT void GetEffectiveBoneTransform(lua_State *l, CBaseEntity &ent, uint32_t boneIdx);
			DLLCLIENT void SendNetEvent(lua_State *l, CBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId, const ::NetPacket &packet);
			DLLCLIENT void SendNetEvent(lua_State *l, CBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId);
		};
	};
};

#endif
