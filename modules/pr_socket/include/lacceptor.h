#ifndef __LACCEPTOR_H__
#define __LACCEPTOR_H__

#include "nwm_boost.h"
#include "luasystem.h"
#include "nwm_packet.h"
#include "luaobj_handler.hpp"

class LTCPAcceptor
	: public std::enable_shared_from_this<LTCPAcceptor>,
	public LuaObjHandler
{
protected:
	std::shared_ptr<tcp::acceptor> m_acceptor;
public:
	LTCPAcceptor(boost::asio::io_service &ioService,const tcp::endpoint &ep);
	~LTCPAcceptor();
	tcp::acceptor &GetAcceptor();
	void Close();
};

namespace Lua
{
	namespace TCPAcceptor
	{
		void Accept(lua_State *l,std::shared_ptr<LTCPAcceptor> &lacceptor,luabind::object callback);
		void Close(lua_State *l,std::shared_ptr<LTCPAcceptor> &lacceptor);
		void GetLocalEndpoint(lua_State *l,std::shared_ptr<LTCPAcceptor> &lacceptor);
	};
};

#endif
