#ifndef __LMYSQL_INSTANCE_HPP__
#define __LMYSQL_INSTANCE_HPP__

#include <memory>
#include <mutex>
#include <queue>
#include <util/functioncallback.h>
#include "lendpoint.h"
#include "lacceptor.h"

struct lua_State;
namespace Lua
{
	struct MySQLInstance
	{
		~MySQLInstance()
		{
			for(auto &wpTcpSocket : tcpSockets)
			{
				if(wpTcpSocket.expired() == true)
					continue;
				wpTcpSocket.lock()->Close();
			}
			for(auto &wpUdpSocket : udpSockets)
			{
				if(wpUdpSocket.expired() == true)
					continue;
				wpUdpSocket.lock()->Close();
			}
			for(auto &wpAcceptor : tcpAcceptors)
			{
				if(wpAcceptor.expired() == true)
					continue;
				wpAcceptor.lock()->Close();
			}
			if(io_service != nullptr)
				io_service->stop();
			io_service = nullptr;
			udp_resolver = nullptr;
			tcp_resolver = nullptr;
			if(thinkCallback.IsValid())
				thinkCallback.Remove();
		}
		std::vector<std::weak_ptr<LTCPSocket>> tcpSockets;
		std::vector<std::weak_ptr<LUDPSocket>> udpSockets;
		std::vector<std::weak_ptr<LTCPAcceptor>> tcpAcceptors;
		std::shared_ptr<boost::asio::io_service> io_service = nullptr;
		std::shared_ptr<udp::resolver> udp_resolver = nullptr;
		std::shared_ptr<tcp::resolver> tcp_resolver = nullptr;

		std::mutex eventMutex;
		std::queue<std::function<void(void)>> eventQueue;
		CallbackHandle thinkCallback;
	};
	Lua::MySQLInstance *get_mysql_instance(lua_State *l);
};

#endif
