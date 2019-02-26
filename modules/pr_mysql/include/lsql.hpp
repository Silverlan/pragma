#ifndef __LSQL_HPP__
#define __LSQL_HPP__

#include <cinttypes>
#include <memory>
#include <string>
#include <luasystem.h>

struct lua_State;

namespace sql {class BaseConnection;};
namespace luabind {class object;};

namespace Lua
{
	class Interface;
	namespace sql
	{
		void register_lua_library(Lua::Interface &l);
		int32_t open(lua_State *l);
		int32_t create_value(lua_State *l);
	};
};

#endif
