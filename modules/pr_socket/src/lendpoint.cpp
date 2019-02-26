#include "lsocket.h"
#include "lendpoint.h"

void Lua::UDPEndpoint::GetAddress(lua_State *l,const udp::endpoint &ep)
{
	auto address = ep.address();
	Lua::PushString(l,address.to_string());
}
void Lua::UDPEndpoint::GetPort(lua_State *l,const udp::endpoint &ep)
{
	Lua::PushInt(l,ep.port());
}
void Lua::UDPEndpoint::GetProtocol(lua_State *l,const udp::endpoint &ep)
{
	Lua::PushInt(l,(ep.protocol() == udp::v4()) ? SOCKET_PROTOCOL_V4 : SOCKET_PROTOCOL_V6);
}

void Lua::TCPEndpoint::GetAddress(lua_State *l,const tcp::endpoint &ep)
{
	auto address = ep.address();
	Lua::PushString(l,address.to_string());
}
void Lua::TCPEndpoint::GetPort(lua_State *l,const tcp::endpoint &ep)
{
	Lua::PushInt(l,ep.port());
}
void Lua::TCPEndpoint::GetProtocol(lua_State *l,const tcp::endpoint &ep)
{
	Lua::PushInt(l,(ep.protocol() == tcp::v4()) ? SOCKET_PROTOCOL_V4 : SOCKET_PROTOCOL_V6);
}