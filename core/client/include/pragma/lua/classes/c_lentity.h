/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_LENTITY_H__
#define __C_LENTITY_H__
#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include <mathutil/glmutil.h>
class EntityHandle;
class NetPacket;
class Scene;
namespace Lua
{
	namespace Entity
	{
		namespace Client
		{
			DLLCLIENT void register_class(luabind::class_<EntityHandle> &classDef);
			DLLCLIENT void IsClientsideOnly(lua_State *l,EntityHandle &hEnt);
			DLLCLIENT void GetClientIndex(lua_State *l,EntityHandle &hEnt);
			DLLCLIENT void GetEffectiveBoneTransform(lua_State *l,EntityHandle &hEnt,uint32_t boneIdx);
			DLLCLIENT void SendNetEvent(lua_State *l,EntityHandle &hEnt,uint32_t protocol,unsigned int eventId,const ::NetPacket &packet);
			DLLCLIENT void SendNetEvent(lua_State *l,EntityHandle &hEnt,uint32_t protocol,unsigned int eventId);

			DLLCLIENT void GetSceneFlags(lua_State *l,EntityHandle &hEnt);
			DLLCLIENT void AddToScene(lua_State *l,EntityHandle &hEnt,::Scene &scene);
			DLLCLIENT void RemoveFromScene(lua_State *l,EntityHandle &hEnt,::Scene &scene);
			DLLCLIENT void IsInScene(lua_State *l,EntityHandle &hEnt,::Scene &scene);
		};
	};
};

#endif