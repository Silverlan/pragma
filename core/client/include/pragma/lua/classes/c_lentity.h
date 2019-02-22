#ifndef __C_LENTITY_H__
#define __C_LENTITY_H__
#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include <mathutil/glmutil.h>
class EntityHandle;
class NetPacket;
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
		};
	};
};

#endif