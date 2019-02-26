#ifndef __LSQL_STATEMENT_HPP__
#define __LSQL_STATEMENT_HPP__

#include <cinttypes>
#include <memory>
#include <string>
#include <luasystem.h>

struct lua_State;

namespace sql {class BaseStatement;};
namespace luabind {class object;};

namespace Lua
{
	class Interface;
	namespace sql
	{
		namespace Statement
		{
			void Step(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement);
			void Step(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,const luabind::object &oCallback);
			void ExecuteQuery(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement);
			void ExecuteQuery(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,const luabind::object &oCallback);
			void Finalize(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement);
			void Finalize(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,const luabind::object &oCallback);
			void Reset(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement);
			void Reset(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,const luabind::object &oCallback);
			void FindColumn(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,const std::string &colName);
			void FindColumn(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,const std::string &colName,const luabind::object &oCallback);

			void IsReadOnly(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column);
			void IsReadOnly(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column,const luabind::object &oCallback);
			void GetType(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column);
			void GetType(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column,const luabind::object &oCallback);
			void GetDouble(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column);
			void GetDouble(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column,const luabind::object &oCallback);
			void GetInt(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column);
			void GetInt(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column,const luabind::object &oCallback);
			void GetInt64(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column);
			void GetInt64(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column,const luabind::object &oCallback);
			void GetText(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column);
			void GetText(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column,const luabind::object &oCallback);
			void GetBlob(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column);
			void GetBlob(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column,const luabind::object &oCallback);
			void GetColumnCount(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement);
			void GetColumnCount(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,const luabind::object &oCallback);
			void GetColumnName(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column);
			void GetColumnName(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column,const luabind::object &oCallback);
			void GetRowCount(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement);
			void GetRowCount(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,const luabind::object &oCallback);
			void GetValue(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column);
			void GetValue(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column,const luabind::object &oCallback);
			void ExecuteQueryAndFetchRows(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,const std::string &sql);
			void ExecuteQueryAndFetchRows(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,const std::string &sql,const luabind::object &oCallback);
		};
	};
};

#endif
