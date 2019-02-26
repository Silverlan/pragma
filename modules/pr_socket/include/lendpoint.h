#ifndef __LENDPOINT_H__
#define __LENDPOINT_H__

#include "nwm_boost.h"
#include "luasystem.h"

LUA_REGISTER_TYPE(TCPEndpoint,tcp::endpoint);

namespace Lua
{
	namespace UDPEndpoint
	{
		void GetAddress(lua_State *l,const udp::endpoint &ep);
		void GetPort(lua_State *l,const udp::endpoint &ep);
		void GetProtocol(lua_State *l,const udp::endpoint &ep);
	};

	namespace TCPEndpoint
	{
		void GetAddress(lua_State *l,const tcp::endpoint &ep);
		void GetPort(lua_State *l,const tcp::endpoint &ep);
		void GetProtocol(lua_State *l,const tcp::endpoint &ep);
	};
};

#endif
