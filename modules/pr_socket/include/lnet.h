#ifndef __LNET_H__
#define __LNET_H__

#include "luasystem.h"

namespace Lua
{
	namespace net
	{
		int create_socket(lua_State *l);
		int create_endpoint(lua_State *l);
		int resolve(lua_State *l);
		int create_tcp_acceptor(lua_State *l);
	};
};

#endif
