#include "lnet.h"
#include "nwm_boost.h"
#include "wvmodule.h"
#include "lsocket.h"
#include "lendpoint.h"
#include "lacceptor.h"
#include "errorcode.h"
#include "lmysql_instance.hpp"
#include <mutex>
#include <queue>
#include <memory>
#include <ishared.hpp>

namespace Lua
{
	namespace net
	{
		static int create_udp_socket(lua_State *l)
		{
			auto *instance = Lua::get_mysql_instance(l);
			if(instance == nullptr)
				return 0;
			auto sock = std::make_shared<LUDPSocket>(*instance->io_service);
			instance->udpSockets.push_back(sock);
			Lua::Push<std::shared_ptr<LUDPSocket>>(l,sock);
			return 1;
		}

		static int create_tcp_socket(lua_State *l)
		{
			auto *instance = Lua::get_mysql_instance(l);
			if(instance == nullptr)
				return 0;
			auto sock = std::make_shared<LTCPSocket>(*instance->io_service);
			instance->tcpSockets.push_back(sock);
			Lua::Push<std::shared_ptr<LTCPSocket>>(l,sock);
			return 1;
		}

		static int create_tcp_endpoint(lua_State *l,int argOffset)
		{
			tcp::endpoint ep;
			if(Lua::IsNumber(l,argOffset))
			{
				auto protocol = Lua::CheckInt(l,argOffset);
				auto port = Lua::CheckInt(l,++argOffset);
				if(protocol = SOCKET_PROTOCOL_V4)
					ep = tcp::endpoint(tcp::v4(),port);
				else
					ep = tcp::endpoint(tcp::v6(),port);
			}
			else
			{
				auto ip = Lua::CheckString(l,argOffset);
				auto port = Lua::CheckInt(l,++argOffset);
				ep = tcp::endpoint(boost::asio::ip::address::from_string(ip),port);
			}
			Lua::Push<tcp::endpoint>(l,ep);
			return 1;
		}

		static int create_udp_endpoint(lua_State *l,int argOffset)
		{
			udp::endpoint ep;
			if(Lua::IsNumber(l,argOffset))
			{
				auto protocol = Lua::CheckInt(l,argOffset);
				auto port = Lua::CheckInt(l,++argOffset);
				if(protocol = SOCKET_PROTOCOL_V4)
					ep = udp::endpoint(udp::v4(),port);
				else
					ep = udp::endpoint(udp::v6(),port);
			}
			else
			{
				auto ip = Lua::CheckString(l,argOffset);
				auto port = Lua::CheckInt(l,++argOffset);
				ep = udp::endpoint(boost::asio::ip::address::from_string(ip),port);
			}
			Lua::Push<udp::endpoint>(l,ep);
			return 1;
		}
	};
};

int Lua::net::create_socket(lua_State *l)
{
	auto protocol = Lua::CheckInt(l,1);
	if(protocol == SOCKET_PROTOCOL_UDP)
		return create_udp_socket(l);
	return create_tcp_socket(l);
}

int Lua::net::create_endpoint(lua_State *l)
{
	auto protocol = Lua::CheckInt(l,1);
	if(protocol == SOCKET_PROTOCOL_UDP)
		return create_udp_endpoint(l,2);
	return create_tcp_endpoint(l,2);
}

int Lua::net::create_tcp_acceptor(lua_State *l)
{
	auto *instance = Lua::get_mysql_instance(l);
	if(instance == nullptr)
		return 0;
	auto *ep = Lua::CheckTCPEndpoint(l,1);
	auto acceptor = std::make_shared<LTCPAcceptor>(*instance->io_service,*ep);
	instance->tcpAcceptors.push_back(acceptor);
	Lua::Push<std::shared_ptr<LTCPAcceptor>>(l,acceptor);
	return 1;
}

int Lua::net::resolve(lua_State *l)
{
	auto *instance = Lua::get_mysql_instance(l);
	if(instance == nullptr)
		return 0;
	auto protocol = Lua::CheckInt(l,1);
	auto ip = Lua::CheckString(l,2);
	std::string service;
	if(Lua::IsNumber(l,3))
	{
		auto port = Lua::CheckInt(l,3);
		service = std::to_string(port);
	}
	else
		service = Lua::CheckString(l,3);
	if(!Lua::IsSet(l,4))
	{
		if(protocol == SOCKET_PROTOCOL_UDP)
		{
			udp::resolver::query query(ip,service);
			boost::system::error_code err;
			auto it = instance->udp_resolver->resolve(query,err);
			Lua::Push<ErrorCode>(l,err);
			if(!err)
			{
				Lua::Push<udp::endpoint>(l,*it);
				return 2;
			}
			return 1;
		}
		tcp::resolver::query query(ip,service);
		boost::system::error_code err;
		auto it = instance->tcp_resolver->resolve(query,err);
		Lua::Push<ErrorCode>(l,err);
		if(!err)
		{
			Lua::Push<tcp::endpoint>(l,*it);
			return 2;
		}
		return 1;
	}
	Lua::CheckFunction(l,4);
	auto callback = luabind::object(luabind::from_stack(l,4));
	if(protocol == SOCKET_PROTOCOL_UDP)
	{
		udp::resolver::query query(ip,service);
		instance->udp_resolver->async_resolve(query,[l,instance,callback](const boost::system::error_code &err,udp::resolver::iterator it) {
			udp::endpoint ep;
			if(!err)
				ep = *it;
			std::lock_guard<std::mutex> guard(instance->eventMutex);
			instance->eventQueue.push([l,callback,err,ep]() {
				callback.push(l);
				Lua::Push<ErrorCode>(l,err);
				if(!err)
				{
					Lua::Push<udp::endpoint>(l,ep);
					ishared::protected_lua_call(l,2,0);
				}
				else
					ishared::protected_lua_call(l,1,0);
			});
		});
	}
	else
	{
		auto query = std::make_shared<tcp::resolver::query>(ip,service);
		instance->tcp_resolver->async_resolve(*query,[l,instance,callback,query](const boost::system::error_code &err,tcp::resolver::iterator it) {
			tcp::endpoint ep;
			if(!err)
				ep = *it;
			std::lock_guard<std::mutex> guard(instance->eventMutex);
			instance->eventQueue.push([l,callback,err,ep]() {
				callback.push(l);
				Lua::Push<ErrorCode>(l,err);
				if(!err)
				{
					Lua::Push<tcp::endpoint>(l,ep);
					ishared::protected_lua_call(l,2,0);
				}
				else
					ishared::protected_lua_call(l,1,0);
			});
		});
	}
	return 0;
}
