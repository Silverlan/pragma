#include "wvmodule.h"
#include "lacceptor.h"
#include "lsocket.h"
#include "errorcode.h"
#include "lmysql_instance.hpp"
#include <mutex>
#include <queue>
#include <ishared.hpp>

LTCPAcceptor::LTCPAcceptor(boost::asio::io_service &ioService,const tcp::endpoint &ep)
{
	m_acceptor = std::make_shared<tcp::acceptor>(ioService,ep);
}

LTCPAcceptor::~LTCPAcceptor()
{
	if(m_acceptor == nullptr || m_acceptor.use_count() > 1)
		return;
	Close();
}

void LTCPAcceptor::Close()
{
	if(m_acceptor == nullptr)
		return;
	m_acceptor->close();
	m_acceptor = nullptr;
}

tcp::acceptor &LTCPAcceptor::GetAcceptor() {return *m_acceptor;}

void Lua::TCPAcceptor::Accept(lua_State *l,::std::shared_ptr<LTCPAcceptor> &lacceptor,luabind::object callback)
{
	auto *instance = Lua::get_mysql_instance(l);
	if(instance == nullptr)
		return;
	Lua::CheckFunction(l,2);
	auto &acceptor = lacceptor->GetAcceptor();
	auto socket = std::make_shared<LTCPSocket>(*instance->io_service);
	instance->tcpSockets.push_back(socket);
	auto &sock = socket->GetSocket();
	auto wpCallback = lacceptor->RegisterObject(callback);
	auto *pAcceptor = lacceptor.get();
	acceptor.async_accept(
		sock,[wpCallback,pAcceptor,l,instance,socket](const boost::system::error_code &err) {
			std::lock_guard<std::mutex> guard(instance->eventMutex);
			instance->eventQueue.push([wpCallback,pAcceptor,l,err,socket]() {
				if(wpCallback.expired() == true)
					return;
				wpCallback.lock()->push(l);
				pAcceptor->FreeObject(*wpCallback.lock());
				auto nargs = 1;
				Lua::Push<ErrorCode>(l,err);
				if(!err)
				{
					Lua::Push<std::shared_ptr<LTCPSocket>>(l,socket);
					nargs++;
				}
				ishared::protected_lua_call(l,nargs,0);
			});
		}
	);
}
void Lua::TCPAcceptor::Close(lua_State *l,::std::shared_ptr<LTCPAcceptor> &lacceptor)
{
	auto &acceptor = lacceptor->GetAcceptor();
	acceptor.close();
}
void Lua::TCPAcceptor::GetLocalEndpoint(lua_State *l,::std::shared_ptr<LTCPAcceptor> &lacceptor)
{
	auto &acceptor = lacceptor->GetAcceptor();
	Lua::Push<tcp::endpoint>(l,acceptor.local_endpoint());
}