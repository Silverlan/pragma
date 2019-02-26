#ifndef __LSQL_CONNECTION_HPP__
#define __LSQL_CONNECTION_HPP__

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
		namespace Connection
		{
			void Execute(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const std::string &sql,const luabind::object &oValues,const luabind::object &oCallback);
			void Execute(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const std::string &sql,const luabind::object &oCallback);
			void Execute(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const std::string &sql);
			void ExecuteQueryAndFetchRows(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const std::string &sql);
			void ExecuteQueryAndFetchRows(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const std::string &sql,const luabind::object &oCallback);
			void ExecuteQueryAndFetchRows(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const std::string &sql,const luabind::object &oValues,const luabind::object &oCallback);
			void Close(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection);
			void GetLastInsertRowId(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection);
			void GetLastInsertRowId(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const luabind::object &oCallback);
			void CreateStatement(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const std::string &sql);
			void CreateStatement(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const std::string &sql,const luabind::object &oCallback);
			void CreatePreparedStatement(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const std::string &sql);
			void CreatePreparedStatement(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const std::string &sql,const luabind::object &oCallback);
			void Wait(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection);
			void CallOnQueueComplete(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const luabind::object &oCallback);

			void GetResultCode(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection);
			void GetResultErrorMessage(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection);
		};
	};
};

#endif
