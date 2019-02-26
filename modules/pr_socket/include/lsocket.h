#ifndef __LSOCKET_H__
#define __LSOCKET_H__

#include "networkmanager/nwm_boost.h"
#include "luasystem.h"
#include "nwm_packet.h"
#include "luaobj_handler.hpp"

#define SOCKET_PROTOCOL_V4 1
#define SOCKET_PROTOCOL_V6 2

#define SOCKET_PROTOCOL_UDP 4
#define SOCKET_PROTOCOL_TCP 8

template<class TSocket>
	class LSocketBase
		: public LuaObjHandler
{
protected:
	boost::asio::io_service &m_ioService;
	std::shared_ptr<TSocket> m_socket;
	std::shared_ptr<boost::asio::deadline_timer> m_timer;
	unsigned int m_tTimeout;
	void CheckDeadline();
public:
	LSocketBase(boost::asio::io_service &ioService);
	virtual ~LSocketBase();
	TSocket &GetSocket() const;
	void SetTimeoutDuration(unsigned int t);
	unsigned int GetTimeoutDuration() const;
	void ResetTimeout();
	void Close();
	bool operator==(const LSocketBase &other) const;
	bool operator!=(const LSocketBase &other) const;
};

template<class TSocket>
	LSocketBase<TSocket>::LSocketBase(boost::asio::io_service &ioService)
		: m_ioService(ioService),m_tTimeout(0)
{
	m_socket = std::make_shared<TSocket>(ioService);
	m_timer = std::make_shared<boost::asio::deadline_timer>(ioService);
	m_timer->expires_at(boost::posix_time::pos_infin);
	CheckDeadline();
}

template<class TSocket>
	LSocketBase<TSocket>::~LSocketBase()
{
	m_timer = nullptr;
	if(m_socket == nullptr || m_socket.use_count() > 1)
		return;
	Close();
}

template<class TSocket>
	void LSocketBase<TSocket>::Close()
{
	if(m_socket == nullptr)
		return;
	m_socket->close();
	m_socket = nullptr;
}

template<class TSocket>
	bool LSocketBase<TSocket>::operator==(const LSocketBase &other) const
	{
		return (m_socket.get() == other.m_socket.get()) ? true : false;
	}

template<class TSocket>
	bool LSocketBase<TSocket>::operator!=(const LSocketBase &other) const
	{
		return (this == other) ? false : true;
	}

template<class TSocket>
	void LSocketBase<TSocket>::CheckDeadline()
{
	if(m_timer == nullptr)
		return;
	if(m_timer->expires_at() <= boost::asio::deadline_timer::traits_type::now())
	{
		m_socket->close();
		m_timer->expires_at(boost::posix_time::pos_infin);
	}
	m_timer->async_wait([this](const boost::system::error_code &err) mutable {
		CheckDeadline();
	});
}

template<class TSocket>
	TSocket &LSocketBase<TSocket>::GetSocket() const {return *m_socket.get();}

template<class TSocket>
	void LSocketBase<TSocket>::SetTimeoutDuration(unsigned int t)
{
	m_tTimeout = t;
	if(t == 0)
		m_timer->expires_at(boost::posix_time::pos_infin);
}
template<class TSocket>
	unsigned int LSocketBase<TSocket>::GetTimeoutDuration() const
{
	return m_tTimeout;
}
template<class TSocket>
	void LSocketBase<TSocket>::ResetTimeout()
{
	if(m_tTimeout == 0)
		return;
	m_timer->expires_from_now(boost::posix_time::seconds(m_tTimeout));
}

class LTCPSocket
	: public LSocketBase<tcp::socket>,
	public std::enable_shared_from_this<LTCPSocket>
{
public:
	LTCPSocket(boost::asio::io_service &ioService);
	std::shared_ptr<boost::asio::streambuf> response;
	using LSocketBase<tcp::socket>::operator==;
	using LSocketBase<tcp::socket>::operator!=;
};

class LUDPSocket
	: public LSocketBase<udp::socket>,
	public std::enable_shared_from_this<LUDPSocket>
{
public:
	LUDPSocket(boost::asio::io_service &ioService);
	using LSocketBase<udp::socket>::operator==;
	using LSocketBase<udp::socket>::operator!=;
};

namespace Lua
{
	namespace UDPSocket
	{
		void Send(lua_State *l,std::shared_ptr<LUDPSocket> &udpSock,const std::string &ip,unsigned short port,NetPacket packet,luabind::object callback);
		void Send(lua_State *l,std::shared_ptr<LUDPSocket> &udpSock,const udp::endpoint &ep,NetPacket packet,luabind::object callback);
		void Receive(lua_State *l,std::shared_ptr<LUDPSocket> &udpSock,unsigned int bytesReceive,luabind::object callback);
		void Bind(lua_State *l,std::shared_ptr<LUDPSocket> &udpSock,const udp::endpoint &ep);
		void Bind(lua_State *l,std::shared_ptr<LUDPSocket> &udpSock,const std::string &ip,unsigned short port,luabind::object callback);
		void Open(lua_State *l,std::shared_ptr<LUDPSocket> &udpSock,int protocol);
		void Open(lua_State *l,std::shared_ptr<LUDPSocket> &udpSock);
		void Close(lua_State *l,std::shared_ptr<LUDPSocket> &udpSock);
		void IsOpen(lua_State *l,std::shared_ptr<LUDPSocket> &udpSock);
		void SetTimeoutDuration(lua_State *l,std::shared_ptr<LUDPSocket> &udpSock,unsigned int t);
		void GetTimeoutDuration(lua_State *l,std::shared_ptr<LUDPSocket> &udpSock);
	};
	namespace TCPSocket
	{
		void Connect(lua_State *l,std::shared_ptr<LTCPSocket> &tcpSock,const tcp::endpoint &ep,luabind::object callback);
		void Connect(lua_State *l,std::shared_ptr<LTCPSocket> &tcpSock,const std::string &ip,unsigned short port,luabind::object callback);
		void Send(lua_State *l,std::shared_ptr<LTCPSocket> &tcpSock,NetPacket packet,luabind::object callback);
		void Bind(lua_State *l,std::shared_ptr<LTCPSocket> &tcpSock,const tcp::endpoint &ep);
		void Bind(lua_State *l,std::shared_ptr<LTCPSocket> &tcpSock,const std::string &ip,unsigned short port,luabind::object callback);
		void Open(lua_State *l,std::shared_ptr<LTCPSocket> &tcpSock,int protocol);
		void Open(lua_State *l,std::shared_ptr<LTCPSocket> &tcpSock);
		void Close(lua_State *l,std::shared_ptr<LTCPSocket> &tcpSock);
		void IsOpen(lua_State *l,std::shared_ptr<LTCPSocket> &tcpSock);
		void SetTimeoutDuration(lua_State *l,std::shared_ptr<LTCPSocket> &tcpSock,unsigned int t);
		void GetTimeoutDuration(lua_State *l,std::shared_ptr<LTCPSocket> &tcpSock);
		void Receive(lua_State *l,std::shared_ptr<LTCPSocket> &tcpSock,unsigned int bytesReceive,luabind::object callback);
		void ReceiveUntil(lua_State *l,std::shared_ptr<LTCPSocket> &tcpSock,const std::string &target,luabind::object callback);
		void ReceiveAtLeast(lua_State *l,std::shared_ptr<LTCPSocket> &tcpSock,unsigned int szData,luabind::object callback);
		void Shutdown(lua_State *l,std::shared_ptr<LTCPSocket> &tcpSock);
		void GetLocalEndpoint(lua_State *l,std::shared_ptr<LTCPSocket> &tcpSock);
		void GetRemoteEndpoint(lua_State *l,std::shared_ptr<LTCPSocket> &tcpSock);
		void GetBytesAvailable(lua_State *l,std::shared_ptr<LTCPSocket> &tcpSock);
	};
};

#endif
