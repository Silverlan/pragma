#ifndef __LSQL_VALUE_HPP__
#define __LSQL_VALUE_HPP__

#include <memory>
#include <sharedutils/datastream.h>
#include <luasystem.h>
#include <ldefinitions.h>
#include "sql_value.hpp"

struct lua_State;
namespace luabind {class object;};

lua_registercheck(SQLValue,::sql::PValue);

namespace Lua
{
	namespace sql
	{
		namespace Value
		{
			void GetType(lua_State *l,::sql::PValue &value);
			void GetDouble(lua_State *l,::sql::PValue &value);
			void GetInt(lua_State *l,::sql::PValue &value);
			void GetInt64(lua_State *l,::sql::PValue &value);
			void GetText(lua_State *l,::sql::PValue &value);
			void GetBlob(lua_State *l,::sql::PValue &value);
			void ToString(lua_State *l,::sql::PValue &value);
		};
	};
};

#endif
