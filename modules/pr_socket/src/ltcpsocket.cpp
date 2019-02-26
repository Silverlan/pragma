#include "lsocket.h"
#include "luasystem.h"
#include "wvmodule.h"
#include "errorcode.h"
#include <mutex>
#include <queue>
#include <ishared.hpp>
#include "lmysql_instance.hpp"

LTCPSocket::LTCPSocket(boost::asio::io_service &ioService)
	: LSocketBase<tcp::socket>(ioService)
{}

static void resolve_tcp(lua_State *l,const std::string &ip,unsigned short port,luabind::object &luaCallback,const std::function<void(const boost::system::error_code&,tcp::resolver::iterator)> &callback)
{
	auto *instance = Lua::get_mysql_instance(l);
	if(instance == nullptr)
		return;
	tcp::resolver::query query(ip,std::to_string(port));
	// TODO
	instance->tcp_resolver->async_resolve(
		query,
		[l,instance,luaCallback,callback](const boost::system::error_code &err,tcp::resolver::iterator it) {
			if(err)
			{
				std::lock_guard<std::mutex> guard(instance->eventMutex);
				instance->eventQueue.push([l,err,luaCallback]() {
					luaCallback.push(l);
					Lua::Push<ErrorCode>(l,err);
					ishared::protected_lua_call(l,1,0);
				});
			}
			else
				callback(err,it);
		}
	);
}
static void connect_tcp(lua_State *l,std::shared_ptr<::LTCPSocket> &tcpSock,const tcp::endpoint &ep,luabind::object &luaCallback)
{
	auto *instance = Lua::get_mysql_instance(l);
	if(instance == nullptr)
		return;
	auto &sock = tcpSock->GetSocket();
	auto wpCallback = tcpSock->RegisterObject(luaCallback);
	auto *pTcpSock = tcpSock.get();
	sock.async_connect(
		ep,[l,pTcpSock,instance,wpCallback](const boost::system::error_code &err) {
			std::lock_guard<std::mutex> guard(instance->eventMutex);
			instance->eventQueue.push([l,pTcpSock,err,wpCallback]() {
				if(wpCallback.expired() == true)
					return;
				wpCallback.lock()->push(l);
				pTcpSock->FreeObject(*wpCallback.lock());
				Lua::Push<ErrorCode>(l,err);
				ishared::protected_lua_call(l,1,0);
			});
		}
	);
}
static void send_tcp(lua_State *l,std::shared_ptr<::LTCPSocket> &tcpSock,NetPacket packet,luabind::object callback)
{
	auto *instance = Lua::get_mysql_instance(l);
	if(instance == nullptr)
		return;
	auto &sock = tcpSock->GetSocket();
	tcpSock->ResetTimeout();
	tcpSock->response = nullptr;
	auto wpCallback = tcpSock->RegisterObject(callback);
	auto *pTcpSock = tcpSock.get();
	sock.async_send(
		boost::asio::buffer(packet->GetData(),packet->GetSize()),
		[l,instance,wpCallback,pTcpSock,packet](const boost::system::error_code& err,std::size_t bytesTransferred) {
			std::lock_guard<std::mutex> guard(instance->eventMutex);
			instance->eventQueue.push([l,wpCallback,pTcpSock,err,bytesTransferred]() {
				if(wpCallback.expired() == true)
					return;
				wpCallback.lock()->push(l);
				pTcpSock->FreeObject(*wpCallback.lock());
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

void Lua::TCPSocket::Send(lua_State *l,std::shared_ptr<::LTCPSocket> &tcpSock,NetPacket packet,luabind::object callback)
{
	Lua::CheckFunction(l,3);
	send_tcp(l,tcpSock,packet,callback);
}

void Lua::TCPSocket::Connect(lua_State *l,std::shared_ptr<::LTCPSocket> &tcpSock,const tcp::endpoint &ep,luabind::object callback)
{
	Lua::CheckFunction(l,3);
	connect_tcp(l,tcpSock,ep,callback);
}
void Lua::TCPSocket::Connect(lua_State *l,std::shared_ptr<::LTCPSocket> &tcpSock,const std::string &ip,unsigned short port,luabind::object callback)
{
	resolve_tcp(l,ip,port,callback,[l,tcpSock,callback](const boost::system::error_code &err,tcp::resolver::iterator it) mutable {
		Lua::Push<ErrorCode>(l,err);
		connect_tcp(l,tcpSock,*it,callback);
	});
}

void Lua::TCPSocket::Bind(lua_State *l,std::shared_ptr<::LTCPSocket> &tcpSock,const tcp::endpoint &ep)
{
	auto &sock = tcpSock->GetSocket();
	boost::system::error_code err;
	sock.bind(ep,err);
	Lua::Push<ErrorCode>(l,err);
}
void Lua::TCPSocket::Bind(lua_State *l,std::shared_ptr<::LTCPSocket> &tcpSock,const std::string &ip,unsigned short port,luabind::object callback)
{
	auto *instance = Lua::get_mysql_instance(l);
	if(instance == nullptr)
		return;
	auto wpCallback = tcpSock->RegisterObject(callback);
	auto *pTcpSock = tcpSock.get();
	resolve_tcp(l,ip,port,callback,[l,instance,wpCallback,pTcpSock,callback](const boost::system::error_code &err,tcp::resolver::iterator it) {
		std::lock_guard<std::mutex> guard(instance->eventMutex);
		tcp::endpoint ep;
		if(!err)
			ep = *it;
		instance->eventQueue.push([l,err,wpCallback,pTcpSock,ep,callback]() {
			if(wpCallback.expired() == true)
				return;
			auto &sock = pTcpSock->GetSocket();
			boost::system::error_code err;
			sock.bind(ep,err);
			wpCallback.lock()->push(l);
			pTcpSock->FreeObject(*wpCallback.lock());
			Lua::Push<ErrorCode>(l,err);
			ishared::protected_lua_call(l,1,0);
		});
	});
}

void Lua::TCPSocket::Open(lua_State *l,std::shared_ptr<::LTCPSocket> &tcpSock,int protocol)
{
	auto &sock = tcpSock->GetSocket();
	boost::system::error_code err;
	if(protocol == SOCKET_PROTOCOL_V4)
		sock.open(tcp::v4(),err);
	else
		sock.open(tcp::v6(),err);
	Lua::Push<ErrorCode>(l,err);
}
void Lua::TCPSocket::Open(lua_State *l,std::shared_ptr<::LTCPSocket> &tcpSock)
{
	Lua::TCPSocket::Open(l,tcpSock,SOCKET_PROTOCOL_V6);
}
void Lua::TCPSocket::Close(lua_State *l,std::shared_ptr<::LTCPSocket> &tcpSock)
{
	auto &sock = tcpSock->GetSocket();
	boost::system::error_code err;
	sock.close(err);
	Lua::Push<ErrorCode>(l,err);
}
void Lua::TCPSocket::IsOpen(lua_State *l,std::shared_ptr<::LTCPSocket> &tcpSock)
{
	auto &sock = tcpSock->GetSocket();
	Lua::PushBool(l,sock.is_open());
}
void Lua::TCPSocket::SetTimeoutDuration(lua_State *l,std::shared_ptr<::LTCPSocket> &tcpSock,unsigned int t)
{
	tcpSock->SetTimeoutDuration(t);
}
void Lua::TCPSocket::GetTimeoutDuration(lua_State *l,std::shared_ptr<::LTCPSocket> &tcpSock)
{
	Lua::PushNumber(l,tcpSock->GetTimeoutDuration());
}

static void read_callback(lua_State *l,std::shared_ptr<::LTCPSocket> &tcpSock,boost::system::error_code err,std::size_t bytesTransferred,const luabind::object &callback,const std::shared_ptr<boost::asio::streambuf> &response)
{
	auto *instance = Lua::get_mysql_instance(l);
	if(instance == nullptr)
		return;
	std::lock_guard<std::mutex> guard(instance->eventMutex);
	NetPacket data;
	auto responseData = response->data();
	auto szResponse = response->size();
	if(err.value() == boost::asio::error::eof && szResponse > 0)
	{
		err = boost::system::error_code();
		bytesTransferred = szResponse;
	}
	for(auto it=boost::asio::buffers_begin(responseData);it!=(boost::asio::buffers_begin(responseData) +bytesTransferred);++it)
		data<<*it;
	data->SetOffset(0);
	data->SetMessageSize(bytesTransferred);
	response->consume(bytesTransferred);

	auto wpCallback = tcpSock->RegisterObject(callback);
	auto *pTcpSock = tcpSock.get();
	instance->eventQueue.push([l,err,wpCallback,pTcpSock,data]() {
		if(wpCallback.expired() == true)
			return;
		wpCallback.lock()->push(l);
		pTcpSock->FreeObject(*wpCallback.lock());

		auto numArgs = 1;
		Lua::Push<ErrorCode>(l,err);
		if(!err)
		{
			Lua::Push<NetPacket>(l,data);
			numArgs++;
		}
		ishared::protected_lua_call(l,numArgs,0);
	});
}

void Lua::TCPSocket::ReceiveAtLeast(lua_State *l,std::shared_ptr<::LTCPSocket> &tcpSock,unsigned int szData,luabind::object callback)
{
	Lua::CheckFunction(l,3);
	auto &sock = tcpSock->GetSocket();
	tcpSock->ResetTimeout();
	if(tcpSock->response == nullptr)
		tcpSock->response = std::make_shared<boost::asio::streambuf>();
	auto response = tcpSock->response;

	auto wpCallback = tcpSock->RegisterObject(callback);
	auto *pTcpSock = tcpSock.get();
	boost::asio::async_read(sock,*response,boost::asio::transfer_at_least(szData),[l,wpCallback,pTcpSock,response](const boost::system::error_code &err,std::size_t bytesTransferred) {
		if(wpCallback.expired() == true)
			return;
		auto callback = wpCallback.lock();
		pTcpSock->FreeObject(*wpCallback.lock());

		read_callback(l,pTcpSock->shared_from_this(),err,bytesTransferred,*callback,response);
	});
}

void Lua::TCPSocket::ReceiveUntil(lua_State *l,std::shared_ptr<::LTCPSocket> &tcpSock,const std::string &target,luabind::object callback)
{
	Lua::CheckFunction(l,3);
	auto &sock = tcpSock->GetSocket();
	tcpSock->ResetTimeout();
	if(tcpSock->response == nullptr)
		tcpSock->response = std::make_shared<boost::asio::streambuf>();
	auto response = tcpSock->response;

	auto wpCallback = tcpSock->RegisterObject(callback);
	auto *pTcpSock = tcpSock.get();
	boost::asio::async_read_until(sock,*response,target,[l,wpCallback,pTcpSock,response](const boost::system::error_code &err,std::size_t bytesTransferred) {
		if(wpCallback.expired() == true)
			return;
		auto callback = wpCallback.lock();
		pTcpSock->FreeObject(*wpCallback.lock());

		read_callback(l,pTcpSock->shared_from_this(),err,bytesTransferred,*callback,response);
	});
}

void Lua::TCPSocket::Receive(lua_State *l,std::shared_ptr<::LTCPSocket> &tcpSock,unsigned int bytesReceive,luabind::object callback)
{
	auto *instance = Lua::get_mysql_instance(l);
	if(instance == nullptr)
		return;
	Lua::CheckFunction(l,3);
	auto &sock = tcpSock->GetSocket();
	NetPacket data;
	data->Resize(bytesReceive);
	tcpSock->ResetTimeout();
	tcpSock->response = nullptr;

	auto wpCallback = tcpSock->RegisterObject(callback);
	auto *pTcpSock = tcpSock.get();
	sock.async_receive(
		boost::asio::buffer(data->GetData(),bytesReceive),
		[l,instance,wpCallback,pTcpSock,data](const boost::system::error_code &err,std::size_t bytesTransferred) {
			std::lock_guard<std::mutex> guard(instance->eventMutex);
			instance->eventQueue.push([l,err,wpCallback,pTcpSock,data]() {
				if(wpCallback.expired() == true)
					return;
				wpCallback.lock()->push(l);
				pTcpSock->FreeObject(*wpCallback.lock());

				auto numArgs = 1;
				Lua::Push<ErrorCode>(l,err);
				if(!err)
				{
					Lua::Push<NetPacket>(l,data);
					numArgs++;
				}
				ishared::protected_lua_call(l,numArgs,0);
			});
		}
	);
}
void Lua::TCPSocket::GetLocalEndpoint(lua_State *l,std::shared_ptr<::LTCPSocket> &tcpSock)
{
	auto &sock = tcpSock->GetSocket();
	Lua::Push<tcp::endpoint>(l,sock.local_endpoint());
}
void Lua::TCPSocket::GetRemoteEndpoint(lua_State *l,std::shared_ptr<::LTCPSocket> &tcpSock)
{
	auto &sock = tcpSock->GetSocket();
	Lua::Push<tcp::endpoint>(l,sock.remote_endpoint());
}
void Lua::TCPSocket::Shutdown(lua_State *l,std::shared_ptr<::LTCPSocket> &tcpSock)
{
	auto shutdownType = Lua::CheckInt(l,1);
	auto &sock = tcpSock->GetSocket();
	sock.shutdown(boost::asio::socket_base::shutdown_type(shutdownType));
}
void Lua::TCPSocket::GetBytesAvailable(lua_State *l,std::shared_ptr<::LTCPSocket> &tcpSock)
{
	auto &sock = tcpSock->GetSocket();
	Lua::PushInt(l,sock.available());
}