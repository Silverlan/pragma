#include "lsocket.h"
#include "luasystem.h"
#include "wvmodule.h"
#include "errorcode.h"
#include "lmysql_instance.hpp"
#include <mutex>
#include <queue>
#include <memory>
#include <ishared.hpp>

LUDPSocket::LUDPSocket(boost::asio::io_service &ioService)
	: LSocketBase<udp::socket>(ioService)
{}

static void send_udp(lua_State *l,std::shared_ptr<::LUDPSocket> &udpSock,const udp::endpoint &ep,NetPacket packet,luabind::object callback)
{
	auto *instance = Lua::get_mysql_instance(l);
	if(instance == nullptr)
		return;
	auto &sock = udpSock->GetSocket();
	udpSock->ResetTimeout();
	auto wpCallback = udpSock->RegisterObject(callback);
	auto *pUdpSock = udpSock.get();
	sock.async_send_to(
		boost::asio::buffer(packet->GetData(),packet->GetSize()),
		ep,[l,instance,pUdpSock,wpCallback,packet](const boost::system::error_code& err,std::size_t bytesTransferred) {
			std::lock_guard<std::mutex> guard(instance->eventMutex);
			instance->eventQueue.push([l,pUdpSock,wpCallback,err,bytesTransferred]() {
				if(wpCallback.expired() == true)
					return;
				wpCallback.lock()->push(l);
				pUdpSock->FreeObject(*wpCallback.lock());
				auto numArgs = 1;
				Lua::Push<ErrorCode>(l,err);
				/*if(!err)
				{
					Lua::PushInt(l,bytesTransferred);
					numArgs++;
				}*/
				ishared::protected_lua_call(l,numArgs,0);
			});
		}
	);
}

static void resolve_udp(lua_State *l,std::shared_ptr<::LUDPSocket> &udpSock,const std::string &ip,unsigned short port,luabind::object &luaCallback,const std::function<void(const boost::system::error_code&,udp::resolver::iterator)> &callback)
{
	auto *instance = Lua::get_mysql_instance(l);
	if(instance == nullptr)
		return;
	udp::resolver::query query(ip,std::to_string(port));
	auto wpCallback = udpSock->RegisterObject(luaCallback);
	auto *pUdpSock = udpSock.get();
	instance->udp_resolver->async_resolve(
		query,
		[l,instance,wpCallback,pUdpSock,callback](const boost::system::error_code &err,udp::resolver::iterator it) {
			if(err)
			{
				std::lock_guard<std::mutex> guard(instance->eventMutex);
				instance->eventQueue.push([l,err,wpCallback,pUdpSock]() {
					if(wpCallback.expired() == true)
						return;
					wpCallback.lock()->push(l);
					pUdpSock->FreeObject(*wpCallback.lock());
					Lua::Push<ErrorCode>(l,err);
					ishared::protected_lua_call(l,1,0);
				});
			}
			else
				callback(err,it);
		}
	);
}

void Lua::UDPSocket::Send(lua_State *l,std::shared_ptr<::LUDPSocket> &udpSock,const std::string &ip,unsigned short port,NetPacket packet,luabind::object callback)
{
	auto *instance = Lua::get_mysql_instance(l);
	if(instance == nullptr)
		return;
	Lua::CheckFunction(l,5);
	auto wpCallback = udpSock->RegisterObject(callback);
	auto *pUdpSock = udpSock.get();
	resolve_udp(l,udpSock,ip,port,callback,[l,instance,wpCallback,pUdpSock,packet,callback](const boost::system::error_code &err,udp::resolver::iterator it) {
			if(wpCallback.expired() == true)
				return;
			std::lock_guard<std::mutex> guard(instance->eventMutex);
			udp::endpoint ep;
			if(!err)
				ep = *it;
			instance->eventQueue.push([l,err,wpCallback,pUdpSock,ep,packet]() {
				if(wpCallback.expired() == true)
					return;
				auto callback = wpCallback.lock();
				pUdpSock->FreeObject(*wpCallback.lock());
				send_udp(l,pUdpSock->shared_from_this(),ep,packet,*callback);
			});
		}
	);
}

void Lua::UDPSocket::Send(lua_State *l,std::shared_ptr<::LUDPSocket> &udpSock,const udp::endpoint &ep,NetPacket packet,luabind::object callback)
{
	Lua::CheckFunction(l,4);
	send_udp(l,udpSock,ep,packet,callback);
}

void Lua::UDPSocket::Receive(lua_State *l,std::shared_ptr<::LUDPSocket> &udpSock,unsigned int bytesReceive,luabind::object callback)
{
	auto *instance = Lua::get_mysql_instance(l);
	if(instance == nullptr)
		return;
	Lua::CheckFunction(l,3);
	auto &sock = udpSock->GetSocket();
	auto ep = std::make_shared<udp::endpoint>();
	NetPacket data;
	data->Resize(bytesReceive);
	udpSock->ResetTimeout();
	auto wpCallback = udpSock->RegisterObject(callback);
	auto *pUdpSock = udpSock.get();
	sock.async_receive_from(
		boost::asio::buffer(data->GetData(),bytesReceive),
		*ep,[l,instance,wpCallback,pUdpSock,data,ep](const boost::system::error_code &err,std::size_t bytesReceived) {
			std::lock_guard<std::mutex> guard(instance->eventMutex);
			instance->eventQueue.push([l,wpCallback,pUdpSock,data,ep,err,bytesReceived]() {
				if(wpCallback.expired() == true)
					return;
				wpCallback.lock()->push(l);
				pUdpSock->FreeObject(*wpCallback.lock());
				auto numArgs = 1;
				Lua::Push<ErrorCode>(l,err);
				if(!err)
				{
					Lua::Push<udp::endpoint>(l,*ep);
					Lua::Push<NetPacket>(l,data);
					numArgs += 2;
				}
				ishared::protected_lua_call(l,numArgs,0);
			});
		}
	);
}

void Lua::UDPSocket::Bind(lua_State *l,std::shared_ptr<::LUDPSocket> &udpSock,const udp::endpoint &ep)
{
	auto &sock = udpSock->GetSocket();
	boost::system::error_code err;
	sock.bind(ep,err);
	Lua::Push<ErrorCode>(l,err);
}
void Lua::UDPSocket::Bind(lua_State *l,std::shared_ptr<::LUDPSocket> &udpSock,const std::string &ip,unsigned short port,luabind::object callback)
{
	auto *instance = Lua::get_mysql_instance(l);
	if(instance == nullptr)
		return;
	std::weak_ptr<LUDPSocket> wpSocket = udpSock;
	auto wpCallback = udpSock->RegisterObject(callback);
	auto *pUdpSock = udpSock.get();
	resolve_udp(l,udpSock,ip,port,callback,[l,instance,pUdpSock,callback](const boost::system::error_code &err,udp::resolver::iterator it) {
		std::lock_guard<std::mutex> guard(instance->eventMutex);
		udp::endpoint ep;
		if(!err)
			ep = *it;
		instance->eventQueue.push([l,err,pUdpSock,ep,callback]() {
			auto &sock = pUdpSock->GetSocket();
			boost::system::error_code err;
			sock.bind(ep,err);
			callback.push(l);
			Lua::Push<ErrorCode>(l,err);
			ishared::protected_lua_call(l,1,0);
		});
	});
}

void Lua::UDPSocket::Open(lua_State *l,std::shared_ptr<::LUDPSocket> &udpSock,int protocol)
{
	auto &sock = udpSock->GetSocket();
	boost::system::error_code err;
	if(protocol == SOCKET_PROTOCOL_V4)
		sock.open(udp::v4(),err);
	else
		sock.open(udp::v6(),err);
	Lua::Push<ErrorCode>(l,err);
}
void Lua::UDPSocket::Open(lua_State *l,std::shared_ptr<::LUDPSocket> &udpSock)
{
	Lua::UDPSocket::Open(l,udpSock,SOCKET_PROTOCOL_V6);
}
void Lua::UDPSocket::Close(lua_State *l,std::shared_ptr<::LUDPSocket> &udpSock)
{
	auto &sock = udpSock->GetSocket();
	boost::system::error_code err;
	sock.shutdown(boost::asio::socket_base::shutdown_both);
	sock.close(err);
	Lua::Push<ErrorCode>(l,err);
}
void Lua::UDPSocket::IsOpen(lua_State *l,std::shared_ptr<::LUDPSocket> &udpSock)
{
	auto &sock = udpSock->GetSocket();
	Lua::PushBool(l,sock.is_open());
}
void Lua::UDPSocket::SetTimeoutDuration(lua_State *l,std::shared_ptr<::LUDPSocket> &udpSock,unsigned int t)
{
	udpSock->SetTimeoutDuration(t);
}
void Lua::UDPSocket::GetTimeoutDuration(lua_State *l,std::shared_ptr<::LUDPSocket> &udpSock)
{
	Lua::PushNumber(l,udpSock->GetTimeoutDuration());
}