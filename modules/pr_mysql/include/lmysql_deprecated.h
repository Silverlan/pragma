
#ifdef __LMYSQL_H__
#define __LMYSQL_H__

#include "luasystem.h"

extern class MySQL;
extern class MySQLStatement;
extern class MySQLResult;
extern class MySQLPreparedStatement;

namespace Lua
{
	namespace mysql
	{
		int connect(lua_State *l);
		int get_error(lua_State *l);

		namespace Connection
		{
			void SetSchema(lua_State *l,MySQL &sql,std::string db);
			void SetSchema(lua_State *l,MySQL &sql,std::string db,luabind::object o);
			void CreateStatement(lua_State *l,MySQL &sql);
			void Release(lua_State *l,MySQL &sql);
			void GetVersion(lua_State *l,MySQL &sql);
			void GetDatabaseProductName(lua_State *l,MySQL &sql);
			void PrepareStatement(lua_State *l,MySQL &sql,std::string query);
		};

		namespace Statement
		{
			void Execute(lua_State *l,MySQLStatement &statement,std::string query);
			void Execute(lua_State *l,MySQLStatement &statement,std::string query,luabind::object o);
			void ExecuteQuery(lua_State *l,MySQLStatement &statement,std::string query);
			void ExecuteQuery(lua_State *l,MySQLStatement &statement,std::string query,luabind::object o);
			void Release(lua_State *l,MySQLStatement &statement);
		};

		namespace PreparedStatement
		{
			void SetBigInt(lua_State *l,MySQLPreparedStatement &prepSmt,unsigned int pIdx,std::string i);
			void SetBoolean(lua_State *l,MySQLPreparedStatement &prepSmt,unsigned int pIdx,bool b);
			void SetDouble(lua_State *l,MySQLPreparedStatement &prepSmt,unsigned int pIdx,double d);
			void SetInt(lua_State *l,MySQLPreparedStatement &prepSmt,unsigned int pIdx,int i);
			void SetInt64(lua_State *l,MySQLPreparedStatement &prepSmt,unsigned int pIdx,int64_t i);
			void SetString(lua_State *l,MySQLPreparedStatement &prepSmt,unsigned int pIdx,std::string s);
			void SetUInt(lua_State *l,MySQLPreparedStatement &prepSmt,unsigned int pIdx,unsigned int i);
			void SetUInt64(lua_State *l,MySQLPreparedStatement &prepSmt,unsigned int pIdx,uint64_t i);
			void Execute(lua_State *l,MySQLPreparedStatement &statement);
			void Execute(lua_State *l,MySQLPreparedStatement &statement,luabind::object o);
			void ExecuteQuery(lua_State *l,MySQLPreparedStatement &statement);
			void ExecuteQuery(lua_State *l,MySQLPreparedStatement &statement,luabind::object o);
		};

		namespace Result
		{
			void FetchNext(lua_State *l,MySQLResult &result);
			void Release(lua_State *l,MySQLResult &result);

			void GetBoolean(lua_State *l,MySQLResult &result,std::string col);
			void GetBoolean(lua_State *l,MySQLResult &result,int col);
			void GetDouble(lua_State *l,MySQLResult &result,std::string col);
			void GetDouble(lua_State *l,MySQLResult &result,int col);
			void GetInt(lua_State *l,MySQLResult &result,std::string col);
			void GetInt(lua_State *l,MySQLResult &result,int col);
			void GetInt64(lua_State *l,MySQLResult &result,std::string col);
			void GetInt64(lua_State *l,MySQLResult &result,int col);
			void GetString(lua_State *l,MySQLResult &result,std::string col);
			void GetString(lua_State *l,MySQLResult &result,int col);
			void GetUInt(lua_State *l,MySQLResult &result,std::string col);
			void GetUInt(lua_State *l,MySQLResult &result,int col);
			void GetUInt64(lua_State *l,MySQLResult &result,std::string col);
			void GetUInt64(lua_State *l,MySQLResult &result,int col);

			void GetType(lua_State *l,MySQLResult &result,std::string col);
		};
	};
};


#endif