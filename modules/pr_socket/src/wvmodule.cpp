#include "lsocket.h"
#include "luasystem.h"
#include "wvmodule.h"
#include "errorcode.h"
#include "lnet.h"
#include "lendpoint.h"
#include "lacceptor.h"
#ifdef _WIN32
#include <Windows.h>
#endif
#include <mutex>
#include <queue>
#include <unordered_map>
#include <luainterface.hpp>
#include <ishared.hpp>
#include <pragma_module.hpp>
#include "util/functioncallback.h"
#include "lmysql_instance.hpp"

#pragma comment(lib,"lua51.lib")
#pragma comment(lib,"luasystem.lib")
#pragma comment(lib,"luabind.lib")
#pragma comment(lib,"vfilesystem.lib")
#pragma comment(lib,"sharedutils.lib")
#pragma comment(lib,"mathutil.lib")
#pragma comment(lib,"networkmanager.lib")
#pragma comment(lib,"shared.lib")
#pragma comment(lib,"ishared.lib")
#pragma comment(lib,"luabind.lib")

static std::unordered_map<lua_State*,std::shared_ptr<Lua::MySQLInstance>> s_instances;
Lua::MySQLInstance *Lua::get_mysql_instance(lua_State *l)
{
	auto it = s_instances.find(l);
	if(it == s_instances.end())
		return nullptr;
	return it->second.get();
}

static bool InitializeSocketModule(std::string *err,Lua::Interface &l)
{
	auto &instance = *s_instances.insert(std::make_pair(l.GetState(),std::make_shared<Lua::MySQLInstance>())).first->second;

	instance.io_service = std::make_shared<boost::asio::io_service>();
	instance.udp_resolver = std::make_shared<udp::resolver>(*instance.io_service);
	instance.tcp_resolver = std::make_shared<tcp::resolver>(*instance.io_service);

	auto *lstate = l.GetState();
	Lua::GetGlobal(lstate,"net"); /* 1 */
	if(!Lua::IsSet(lstate,-1) || !Lua::IsTable(lstate,-1))
	{
		Lua::Pop(lstate,1); /* 0 */
		return false;
	}
	auto top = Lua::GetStackTop(lstate);
	Lua::PushString(lstate,"create_socket");
	Lua::PushCFunction(lstate,Lua::net::create_socket);
	Lua::SetTableValue(lstate,top);

	Lua::PushString(lstate,"create_endpoint");
	Lua::PushCFunction(lstate,Lua::net::create_endpoint);
	Lua::SetTableValue(lstate,top);

	Lua::PushString(lstate,"resolve");
	Lua::PushCFunction(lstate,Lua::net::resolve);
	Lua::SetTableValue(lstate,top);

	Lua::PushString(lstate,"create_tcp_acceptor");
	Lua::PushCFunction(lstate,Lua::net::create_tcp_acceptor);
	Lua::SetTableValue(lstate,top);

	Lua::Pop(lstate,1); /* 0 */

	auto &eventMutex = instance.eventMutex;
	auto &eventQueue = instance.eventQueue;
	instance.thinkCallback = FunctionCallback<void>::Create([&eventMutex,&eventQueue,&instance]() {
		eventMutex.lock();
		if(!eventQueue.empty())
		{
			auto cpyQueue = eventQueue;
			eventQueue = {};
			eventMutex.unlock();
			while(!cpyQueue.empty())
			{
				auto &f = cpyQueue.front();
				f();
				cpyQueue.pop();
			}
		}
		else
			eventMutex.unlock();
		instance.io_service->reset();
		instance.io_service->poll();
	});
	ishared::add_callback(lstate,ishared::Callback::Think,instance.thinkCallback);

	auto &modNet = l.RegisterLibrary("net");
	auto classDefUDPSocket = luabind::class_<LUDPSocket>("UDPSocket");
	classDefUDPSocket.def("Send",(void(*)(lua_State*,LUDPSocket&,const std::string&,unsigned short,NetPacket,luabind::object))&Lua::UDPSocket::Send);
	classDefUDPSocket.def("Send",(void(*)(lua_State*,LUDPSocket&,const udp::endpoint&,NetPacket,luabind::object))&Lua::UDPSocket::Send);
	classDefUDPSocket.def("Receive",&Lua::UDPSocket::Receive);
	classDefUDPSocket.def("Bind",(void(*)(lua_State*,LUDPSocket&,const udp::endpoint&))&Lua::UDPSocket::Bind);
	classDefUDPSocket.def("Bind",(void(*)(lua_State*,LUDPSocket&,const std::string&,unsigned short,luabind::object))&Lua::UDPSocket::Bind);
	classDefUDPSocket.def("Open",(void(*)(lua_State*,LUDPSocket&,int))&Lua::UDPSocket::Open);
	classDefUDPSocket.def("Open",(void(*)(lua_State*,LUDPSocket&))&Lua::UDPSocket::Open);
	classDefUDPSocket.def("Close",&Lua::UDPSocket::Close);
	classDefUDPSocket.def("IsOpen",&Lua::UDPSocket::IsOpen);
	classDefUDPSocket.def("SetTimeoutDuration",&Lua::UDPSocket::SetTimeoutDuration);
	classDefUDPSocket.def("GetTimeoutDuration",&Lua::UDPSocket::GetTimeoutDuration);
	//classDefUDPSocket.def(luabind::self == luabind::other<LUDPSocket>());
	modNet[classDefUDPSocket];

	auto classDefTCPSocket = luabind::class_<LTCPSocket>("TCPSocket");
	classDefTCPSocket.def("Connect",(void(*)(lua_State*,LTCPSocket&,const tcp::endpoint&,luabind::object))&Lua::TCPSocket::Connect);
	classDefTCPSocket.def("Connect",(void(*)(lua_State*,LTCPSocket&,const std::string&,unsigned short,luabind::object))&Lua::TCPSocket::Connect);
	classDefTCPSocket.def("Bind",(void(*)(lua_State*,LTCPSocket&,const tcp::endpoint&))&Lua::TCPSocket::Bind);
	classDefTCPSocket.def("Bind",(void(*)(lua_State*,LTCPSocket&,const std::string&,unsigned short,luabind::object))&Lua::TCPSocket::Bind);
	classDefTCPSocket.def("Open",(void(*)(lua_State*,LTCPSocket&,int))&Lua::TCPSocket::Open);
	classDefTCPSocket.def("Open",(void(*)(lua_State*,LTCPSocket&))&Lua::TCPSocket::Open);
	classDefTCPSocket.def("Close",&Lua::TCPSocket::Close);
	classDefTCPSocket.def("IsOpen",&Lua::TCPSocket::IsOpen);
	classDefTCPSocket.def("SetTimeoutDuration",&Lua::TCPSocket::SetTimeoutDuration);
	classDefTCPSocket.def("GetTimeoutDuration",&Lua::TCPSocket::GetTimeoutDuration);
	classDefTCPSocket.def("Send",&Lua::TCPSocket::Send);
	classDefTCPSocket.def("Receive",&Lua::TCPSocket::Receive);
	classDefTCPSocket.def("ReceiveUntil",&Lua::TCPSocket::ReceiveUntil);
	classDefTCPSocket.def("ReceiveAtLeast",&Lua::TCPSocket::ReceiveAtLeast);
	classDefTCPSocket.def("Shutdown",&Lua::TCPSocket::Shutdown);
	classDefTCPSocket.def("GetLocalEndpoint",&Lua::TCPSocket::GetLocalEndpoint);
	classDefTCPSocket.def("GetRemoteEndpoint",&Lua::TCPSocket::GetRemoteEndpoint);
	classDefTCPSocket.def("GetBytesAvailable",&Lua::TCPSocket::GetBytesAvailable);
	//classDefTCPSocket.def(luabind::self == luabind::other<LTCPSocket>());
	modNet[classDefTCPSocket];

	auto classDefTCPAcceptor = luabind::class_<LTCPAcceptor>("TCPAcceptor");
	classDefTCPAcceptor.def("Accept",&Lua::TCPAcceptor::Accept);
	classDefTCPAcceptor.def("Close",&Lua::TCPAcceptor::Close);
	classDefTCPAcceptor.def("GetLocalEndpoint",&Lua::TCPAcceptor::GetLocalEndpoint);
	modNet[classDefTCPAcceptor];

	auto classDefUDPEndpoint = luabind::class_<udp::endpoint>("UDPEndpoint");
	classDefUDPEndpoint.def("GetAddress",&Lua::UDPEndpoint::GetAddress);
	classDefUDPEndpoint.def("GetPort",&Lua::UDPEndpoint::GetPort);
	classDefUDPEndpoint.def("GetProtocol",&Lua::UDPEndpoint::GetProtocol);
	classDefUDPEndpoint.def(luabind::self == luabind::other<udp::endpoint>());
	modNet[classDefUDPEndpoint];

	auto classDefTCPEndpoint = luabind::class_<tcp::endpoint>("TCPEndpoint");
	classDefTCPEndpoint.def("GetAddress",&Lua::TCPEndpoint::GetAddress);
	classDefTCPEndpoint.def("GetPort",&Lua::TCPEndpoint::GetPort);
	classDefTCPEndpoint.def("GetProtocol",&Lua::TCPEndpoint::GetProtocol);
	classDefTCPEndpoint.def(luabind::self == luabind::other<tcp::endpoint>());
	modNet[classDefTCPEndpoint];

	// Enums
	Lua::RegisterLibraryEnums(lstate,"net",{
		// Protocol
		{"SOCKET_PROTOCOL_V4",SOCKET_PROTOCOL_V4},
		{"SOCKET_PROTOCOL_V6",SOCKET_PROTOCOL_V6},
		{"SOCKET_PROTOCOL_UDP",SOCKET_PROTOCOL_UDP},
		{"SOCKET_PROTOCOL_TCP",SOCKET_PROTOCOL_TCP},

		// Shutdown Type
		{"SOCKET_SHUTDOWN_BOTH",boost::asio::socket_base::shutdown_both},
		{"SOCKET_SHUTDOWN_RECEIVE",boost::asio::socket_base::shutdown_receive},
		{"SOCKET_SHUTDOWN_SEND",boost::asio::socket_base::shutdown_send},

		// Error
		{"SOCKET_ERROR_ACCESS_DENIED",boost::asio::error::access_denied},	
		{"SOCKET_ERROR_ADDRESS_FAMILY_NOT_SUPPORTED",boost::asio::error::address_family_not_supported},	
		{"SOCKET_ERROR_ADDRESS_IN_USE",boost::asio::error::address_in_use},	
		{"SOCKET_ERROR_ALREADY_CONNECTED",boost::asio::error::already_connected},	
		{"SOCKET_ERROR_ALREADY_STARTED",boost::asio::error::already_started},	
		{"SOCKET_ERROR_BROKEN_PIPE",boost::asio::error::broken_pipe},	
		{"SOCKET_ERROR_CONNECTION_ABORTED",boost::asio::error::connection_aborted},	
		{"SOCKET_ERROR_CONNECTION_REFUSED",boost::asio::error::connection_refused},	
		{"SOCKET_ERROR_CONNECTION_RESET",boost::asio::error::connection_reset},	
		{"SOCKET_ERROR_BAD_DESCRIPTOR",boost::asio::error::bad_descriptor},	
		{"SOCKET_ERROR_FAULT",boost::asio::error::fault},	
		{"SOCKET_ERROR_HOST_UNREACHABLE",boost::asio::error::host_unreachable},	
		{"SOCKET_ERROR_IN_PROGRESS",boost::asio::error::in_progress},	
		{"SOCKET_ERROR_INTERRUPTED",boost::asio::error::interrupted},	
		{"SOCKET_ERROR_INVALID_ARGUMENT",boost::asio::error::invalid_argument},	
		{"SOCKET_ERROR_MESSAGE_SIZE",boost::asio::error::message_size},	
		{"SOCKET_ERROR_NAME_TOO_LONG",boost::asio::error::name_too_long},	
		{"SOCKET_ERROR_NETWORK_DOWN",boost::asio::error::network_down},	
		{"SOCKET_ERROR_NETWORK_RESET",boost::asio::error::network_reset},	
		{"SOCKET_ERROR_NETWORK_UNREACHABLE",boost::asio::error::network_unreachable},	
		{"SOCKET_ERROR_NO_DESCRIPTORS",boost::asio::error::no_descriptors},	
		{"SOCKET_ERROR_NO_BUFFER_SPACE",boost::asio::error::no_buffer_space},	
		{"SOCKET_ERROR_NO_MEMORY",boost::asio::error::no_memory},	
		{"SOCKET_ERROR_NO_PERMISSION",boost::asio::error::no_permission},	
		{"SOCKET_ERROR_NO_PROTOCOL_OPTION",boost::asio::error::no_protocol_option},	
		{"SOCKET_ERROR_NOT_CONNECTED",boost::asio::error::not_connected},	
		{"SOCKET_ERROR_NOT_SOCKET",boost::asio::error::not_socket},	
		{"SOCKET_ERROR_OPERATION_ABORTED",boost::asio::error::operation_aborted},	
		{"SOCKET_ERROR_OPERATION_NOT_SUPPORTED",boost::asio::error::operation_not_supported},	
		{"SOCKET_ERROR_SHUT_DOWN",boost::asio::error::shut_down},
	
		{"SOCKET_ERROR_TIMED_OUT",boost::asio::error::timed_out},	
		{"SOCKET_ERROR_TRY_AGAIN",boost::asio::error::try_again},	
		{"SOCKET_ERROR_WOULD_BLOCK",boost::asio::error::would_block},	
		{"SOCKET_ERROR_HOST_NOT_FOUND",boost::asio::error::host_not_found},	
		{"SOCKET_ERROR_HOST_NOT_FOUND_TRY_AGAIN",boost::asio::error::host_not_found_try_again},	
		{"SOCKET_ERROR_NO_DATA",boost::asio::error::no_data},	
		{"SOCKET_ERROR_NO_RECOVERY",boost::asio::error::no_recovery},	
		{"SOCKET_ERROR_SERVICE_NOT_FOUND",boost::asio::error::service_not_found},	
		{"SOCKET_ERROR_SOCKET_TYPE_NOT_SUPPORTED",boost::asio::error::socket_type_not_supported},	
		{"SOCKET_ERROR_ALREADY_OPEN",boost::asio::error::already_open},	
		{"SOCKET_ERROR_EOF",boost::asio::error::eof},	
		{"SOCKET_ERROR_NOT_FOUND",boost::asio::error::not_found},	
		{"SOCKET_ERROR_FD_SET_FAILURE",boost::asio::error::fd_set_failure}
	});
	//
	return true;
}

extern "C"
{
	void PRAGMA_EXPORT pragma_initialize_lua(Lua::Interface &l)
	{
		std::string err;
		if(InitializeSocketModule(&err,l) == false)
			std::cout<<"WARNING: Unable to initialize MySQL module: "<<err<<std::endl;
	}
	void PRAGMA_EXPORT pragma_post_terminate_lua(const std::string &luaIdentifier,const void *luaState)
	{
		auto it = s_instances.find(const_cast<lua_State*>(reinterpret_cast<const lua_State*>(luaState)));
		if(it == s_instances.end())
			return;
		s_instances.erase(it);
	}
};
