/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_LENTITY_H__
#define __S_LENTITY_H__
#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include "pragma/lua/classes/ldef_entity.h"

namespace pragma::networking {class TargetRecipientFilter;};
namespace Lua
{
	namespace Entity
	{
		namespace Server
		{
			DLLSERVER void register_class(luabind::class_<EntityHandle> &classDef);
			DLLSERVER void IsShared(lua_State *l,EntityHandle &hEnt);
			DLLSERVER void SetShared(lua_State *l,EntityHandle &hEnt,bool b);
			DLLSERVER void SendNetEvent(lua_State *l,EntityHandle &hEnt,uint32_t protocol,unsigned int eventId,::NetPacket packet,pragma::networking::TargetRecipientFilter &rp);
			DLLSERVER void SendNetEvent(lua_State *l,EntityHandle &hEnt,uint32_t protocol,unsigned int eventId,::NetPacket packet);
			DLLSERVER void SendNetEvent(lua_State *l,EntityHandle &hEnt,uint32_t protocol,unsigned int eventId,pragma::networking::TargetRecipientFilter &rp);
			DLLSERVER void SendNetEvent(lua_State *l,EntityHandle &hEnt,uint32_t protocol,unsigned int eventId);
			DLLSERVER void IsSynchronized(lua_State *l,EntityHandle &hEnt);
			DLLSERVER void SetSynchronized(lua_State *l,EntityHandle &hEnt,Bool b);
			DLLSERVER void SetSnapshotDirty(lua_State *l,EntityHandle &hEnt,bool b);
		};
	};
};

#endif