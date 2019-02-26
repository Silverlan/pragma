#if 0
#include "wvmodule.h"
#include "lmysql.h"
#include "mysql.h"
#include <sharedutils/functioncallback.h>
#include <cppconn/metadata.h>
#include <ishared.hpp>

namespace Lua
{
	namespace mysql
	{
		namespace impl
		{
			static void ConnectionCallback(lua_State *lua,MySQL *sql,int function)
			{
				if(function != -1)
				{
					Lua::PushRegistryValue(lua,function);
					int numArgs = 0;
					if(sql != NULL)
					{
						numArgs = 1;
						Lua::Push<MySQL*>(lua,sql);
					}
					ishared::protected_lua_call(lua,numArgs,0);
					Lua::ReleaseReference(lua,function);
				}
			}
			static void SetSchemaCallback(lua_State *lua,MySQL *sql,bool result,int function)
			{
				if(function != -1)
				{
					Lua::PushRegistryValue(lua,function);
					int numArgs = 1;
					Lua::PushBool(lua,result);
					ishared::protected_lua_call(lua,numArgs,0);
					Lua::ReleaseReference(lua,function);
				}
			}
			static void ExecuteCallback(lua_State *lua,MySQL *sql,bool result,int function)
			{
				if(function != -1)
				{
					Lua::PushRegistryValue(lua,function);
					int numArgs = 1;
					Lua::PushBool(lua,result);
					ishared::protected_lua_call(lua,numArgs,0);
					Lua::ReleaseReference(lua,function);
				}
			}
			static void ExecuteQueryCallback(lua_State *lua,MySQL *sql,MySQLResult *result,int function)
			{
				if(function != -1)
				{
					Lua::PushRegistryValue(lua,function);
					int numArgs = 0;
					if(result != NULL)
					{
						numArgs = 1;
						Lua::Push<MySQLResult*>(lua,result);
					}
					ishared::protected_lua_call(lua,numArgs,0);
					Lua::ReleaseReference(lua,function);
				}
			}
		};
	};
};

/////////////////////////////////////////

int Lua::mysql::connect(lua_State *l)
{
	std::string host = Lua::CheckString(l,1);
	std::string user = Lua::CheckString(l,2);
	std::string pass = Lua::CheckString(l,3);
	std::string db = Lua::CheckString(l,4);
	CallbackHandle cb;
	if(Lua::IsSet(l,5))
	{
		Lua::CheckFunction(l,5);
		Lua::PushValue(l,5);
		int callback = Lua::CreateReference(l);
		cb = FunctionCallback<void,MySQL*>::Create(std::bind(&Lua::mysql::impl::ConnectionCallback,l,std::placeholders::_1,callback));
	}
	MySQL *sql = MySQL::Connect(host,user,pass,db,cb);
	if(sql == NULL)
		return 0;
	Lua::Push<MySQL*>(l,sql);
	return 1;
}
int Lua::mysql::get_error(lua_State *l)
{
	Lua::PushString(l,MySQL::LAST_ERROR);
	MySQL::LAST_ERROR = "";
	return 1;
}

void Lua::mysql::Connection::SetSchema(lua_State *l,MySQL &sql,std::string db) {sql.SetSchema(db);}
void Lua::mysql::Connection::SetSchema(lua_State *l,MySQL &sql,std::string db,luabind::object o)
{
	Lua::CheckFunction(l,3);
	Lua::PushValue(l,3);
	int callback = Lua::CreateReference(l);
	auto cb = FunctionCallback<void,MySQL*,bool>::Create(std::bind(&Lua::mysql::impl::SetSchemaCallback,l,std::placeholders::_1,std::placeholders::_2,callback));
	sql.SetSchema(db,cb);
}
void Lua::mysql::Connection::CreateStatement(lua_State *l,MySQL &sql)
{
	MySQLStatement *statement = sql.CreateStatement();
	if(statement == NULL)
		return;
	Lua::Push<MySQLStatement*>(l,statement);
}
void Lua::mysql::Connection::Release(lua_State *l,MySQL &sql)
{
	delete &sql;
}
void Lua::mysql::Connection::GetVersion(lua_State *l,MySQL &sql)
{
	sql::DatabaseMetaData *meta = sql.GetConnection()->getMetaData();
	int major = meta->getDatabaseMajorVersion();
	int minor = meta->getDatabaseMinorVersion();
	Lua::PushInt(l,major);
	Lua::PushInt(l,minor);
}
void Lua::mysql::Connection::GetDatabaseProductName(lua_State *l,MySQL &sql)
{
	sql::DatabaseMetaData *meta = sql.GetConnection()->getMetaData();
	std::string productName = meta->getDatabaseProductName();
	Lua::PushString(l,productName);
}
void Lua::mysql::Connection::PrepareStatement(lua_State *l,MySQL &sql,std::string query)
{
	MySQLPreparedStatement *smt = sql.PrepareStatement(query);
	if(smt == NULL)
		return;
	Lua::Push<MySQLPreparedStatement*>(l,smt);
}

void Lua::mysql::PreparedStatement::SetBigInt(lua_State *l,MySQLPreparedStatement &prepSmt,unsigned int pIdx,std::string i) {prepSmt.SetBigInt(pIdx,i);}
void Lua::mysql::PreparedStatement::SetBoolean(lua_State *l,MySQLPreparedStatement &prepSmt,unsigned int pIdx,bool b) {prepSmt.SetBoolean(pIdx,b);}
void Lua::mysql::PreparedStatement::SetDouble(lua_State *l,MySQLPreparedStatement &prepSmt,unsigned int pIdx,double d) {prepSmt.SetDouble(pIdx,d);}
void Lua::mysql::PreparedStatement::SetInt(lua_State *l,MySQLPreparedStatement &prepSmt,unsigned int pIdx,int i) {prepSmt.SetInt(pIdx,i);}
void Lua::mysql::PreparedStatement::SetInt64(lua_State *l,MySQLPreparedStatement &prepSmt,unsigned int pIdx,int64_t i) {prepSmt.SetInt64(pIdx,i);}
void Lua::mysql::PreparedStatement::SetString(lua_State *l,MySQLPreparedStatement &prepSmt,unsigned int pIdx,std::string s) {prepSmt.SetString(pIdx,s);}
void Lua::mysql::PreparedStatement::SetUInt(lua_State *l,MySQLPreparedStatement &prepSmt,unsigned int pIdx,unsigned int i) {prepSmt.SetUInt(pIdx,i);}
void Lua::mysql::PreparedStatement::SetUInt64(lua_State *l,MySQLPreparedStatement &prepSmt,unsigned int pIdx,uint64_t i) {prepSmt.SetUInt64(pIdx,i);}
void Lua::mysql::PreparedStatement::Execute(lua_State *l,MySQLPreparedStatement &statement)
{
	bool result = statement.Execute();
	Lua::PushBool(l,result);
}
void Lua::mysql::PreparedStatement::Execute(lua_State *l,MySQLPreparedStatement &statement,luabind::object o)
{
	Lua::CheckFunction(l,2);
	Lua::PushValue(l,2);
	int callback = Lua::CreateReference(l);
	auto cb = FunctionCallback<void,MySQL*,bool>::Create(std::bind(&Lua::mysql::impl::ExecuteCallback,l,std::placeholders::_1,std::placeholders::_2,callback));
	statement.Execute(cb);
}
void Lua::mysql::PreparedStatement::ExecuteQuery(lua_State *l,MySQLPreparedStatement &statement)
{
	MySQLResult *res = statement.ExecuteQuery();
	if(res == NULL)
		return;
	Lua::Push<MySQLResult*>(l,res);
}
void Lua::mysql::PreparedStatement::ExecuteQuery(lua_State *l,MySQLPreparedStatement &statement,luabind::object o)
{
	Lua::CheckFunction(l,2);
	Lua::PushValue(l,2);
	int callback = Lua::CreateReference(l);
	auto cb = FunctionCallback<void,MySQL*,MySQLResult*>::Create(std::bind(&Lua::mysql::impl::ExecuteQueryCallback,l,std::placeholders::_1,std::placeholders::_2,callback));
	statement.ExecuteQuery(cb);
}

void Lua::mysql::Statement::Execute(lua_State *l,MySQLStatement &statement,std::string query)
{
	bool result = statement.Execute(query);
	Lua::PushBool(l,result);
}
void Lua::mysql::Statement::Execute(lua_State *l,MySQLStatement &statement,std::string query,luabind::object o)
{
	Lua::CheckFunction(l,3);
	Lua::PushValue(l,3);
	int callback = Lua::CreateReference(l);
	auto cb = FunctionCallback<void,MySQL*,bool>::Create(std::bind(&Lua::mysql::impl::ExecuteCallback,l,std::placeholders::_1,std::placeholders::_2,callback));
	statement.Execute(query,cb);
}
void Lua::mysql::Statement::ExecuteQuery(lua_State *l,MySQLStatement &statement,std::string query)
{
	MySQLResult *res = statement.ExecuteQuery(query);
	if(res == NULL)
		return;
	Lua::Push<MySQLResult*>(l,res);
}
void Lua::mysql::Statement::ExecuteQuery(lua_State *l,MySQLStatement &statement,std::string query,luabind::object o)
{
	Lua::CheckFunction(l,3);
	Lua::PushValue(l,3);
	int callback = Lua::CreateReference(l);
	auto cb = FunctionCallback<void,MySQL*,MySQLResult*>::Create(std::bind(&Lua::mysql::impl::ExecuteQueryCallback,l,std::placeholders::_1,std::placeholders::_2,callback));
	statement.ExecuteQuery(query,cb);
}
void Lua::mysql::Statement::Release(lua_State *l,MySQLStatement &statement)
{
	delete &statement;
}

void Lua::mysql::Result::GetBoolean(lua_State *l,MySQLResult &result,std::string col)
{
	bool r;
	bool b = result.GetBoolean(&r,col);
	if(b == false)
		return;
	Lua::PushBool(l,r);
}
void Lua::mysql::Result::GetBoolean(lua_State *l,MySQLResult &result,int col)
{
	bool r;
	bool b = result.GetBoolean(&r,col);
	if(b == false)
		return;
	Lua::PushBool(l,r);
}
void Lua::mysql::Result::GetDouble(lua_State *l,MySQLResult &result,std::string col)
{
	long double r;
	bool b = result.GetDouble(&r,col);
	if(b == false)
		return;
	Lua::PushNumber(l,double(r));
}
void Lua::mysql::Result::GetDouble(lua_State *l,MySQLResult &result,int col)
{
	long double r;
	bool b = result.GetDouble(&r,col);
	if(b == false)
		return;
	Lua::PushNumber(l,double(r));
}
void Lua::mysql::Result::GetInt(lua_State *l,MySQLResult &result,std::string col)
{
	int r;
	bool b = result.GetInt(&r,col);
	if(b == false)
		return;
	Lua::PushInt(l,r);
}
void Lua::mysql::Result::GetInt(lua_State *l,MySQLResult &result,int col)
{
	int r;
	bool b = result.GetInt(&r,col);
	if(b == false)
		return;
	Lua::PushInt(l,r);
}
void Lua::mysql::Result::GetInt64(lua_State *l,MySQLResult &result,std::string col)
{
	int64_t r;
	bool b = result.GetInt64(&r,col);
	if(b == false)
		return;
	Lua::PushInt(l,r);
}
void Lua::mysql::Result::GetInt64(lua_State *l,MySQLResult &result,int col)
{
	int64_t r;
	bool b = result.GetInt64(&r,col);
	if(b == false)
		return;
	Lua::PushInt(l,r);
}
void Lua::mysql::Result::GetString(lua_State *l,MySQLResult &result,std::string col)
{
	std::string r;
	bool b = result.GetString(&r,col);
	if(b == false)
		return;
	Lua::PushString(l,r);
}
void Lua::mysql::Result::GetString(lua_State *l,MySQLResult &result,int col)
{
	std::string r;
	bool b = result.GetString(&r,col);
	if(b == false)
		return;
	Lua::PushString(l,r);
}
void Lua::mysql::Result::GetUInt(lua_State *l,MySQLResult &result,std::string col)
{
	unsigned int r;
	bool b = result.GetUInt(&r,col);
	if(b == false)
		return;
	Lua::PushInt(l,r);
}
void Lua::mysql::Result::GetUInt(lua_State *l,MySQLResult &result,int col)
{
	unsigned int r;
	bool b = result.GetUInt(&r,col);
	if(b == false)
		return;
	Lua::PushInt(l,r);
}
void Lua::mysql::Result::GetUInt64(lua_State *l,MySQLResult &result,std::string col)
{
	uint64_t r;
	bool b = result.GetUInt64(&r,col);
	if(b == false)
		return;
	Lua::PushInt(l,r);
}
void Lua::mysql::Result::GetUInt64(lua_State *l,MySQLResult &result,int col)
{
	uint64_t r;
	bool b = result.GetUInt64(&r,col);
	if(b == false)
		return;
	Lua::PushInt(l,r);
}
void Lua::mysql::Result::Release(lua_State *l,MySQLResult &result)
{
	delete &result;
}

void Lua::mysql::Result::GetType(lua_State *l,MySQLResult &result,std::string col);

void Lua::mysql::Result::FetchNext(lua_State *l,MySQLResult &result)
{
	Lua::PushBool(l,result.FetchNext());
}
#endif