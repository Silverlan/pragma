#ifndef __C_LNETPACKET_H__
#define __C_LNETPACKET_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include "pragma/lua/classes/ldef_netpacket.h"

namespace Lua
{
	namespace NetPacket
	{
		namespace Client
		{
			DLLCLIENT void register_class(luabind::class_<::NetPacket> &classDef);
			DLLCLIENT void ReadUniqueEntity(lua_State *l,::NetPacket &packet,luabind::object o);
		};
	};
};

#endif