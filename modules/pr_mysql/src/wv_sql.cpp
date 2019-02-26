#include <mathutil/umath.h>
#include <luainterface.hpp>
#include <sqlite3.h>
#include <pragma_module.hpp>
#include "lsql.hpp"

#pragma comment(lib,"lua51.lib")
#pragma comment(lib,"luasystem.lib")
#pragma comment(lib,"mysqlcppconn.lib")
#pragma comment(lib,"luabind.lib")
#pragma comment(lib,"vfilesystem.lib")
#pragma comment(lib,"sharedutils.lib")
#pragma comment(lib,"ishared.lib")
#pragma comment(lib,"mathutil.lib")

namespace Lua
{
	namespace sql
	{
		//int32_t result_to_string(lua_State *l);

		namespace Connection
		{
			/*void PrepareStatement(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &statement);
			void PrepareStatement(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &statement,const luabind::object &oAsyncCallback);
			void OpenBlob(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &tableName,const std::string &columnName,int64_t rowId,bool bReadOnly);
			void OpenBlob(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &tableName,const std::string &columnName,int64_t rowId);
			void OpenBlob(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &dbName,const std::string &tableName,const std::string &columnName,int64_t rowId,bool bReadOnly);
			void OpenBlob(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &dbName,const std::string &tableName,const std::string &columnName,int64_t rowId);
			void Exec(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &sql,const luabind::object &oCallback);
			void Exec(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &sql);
			void GetResult(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection);
			void GetResultMessage(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection);
			void IsDatabaseReadOnly(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &dbName);
			void DatabaseExists(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &dbName);

			void CreateFunction(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &funcName,int32_t argCount,bool bDeterministic,const luabind::object &oXFunc);
			void CreateFunction(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &funcName,int32_t argCount,bool bDeterministic,const luabind::object &oXStep,const luabind::object &oXFinal);
			void DeleteFunction(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &funcName,int32_t argCount,bool bDeterministic);
		*/
		};

		namespace Statement
		{
			/*void Finalize(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement);
			void Step(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement);
			void Step(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,const luabind::object &oAsynCallback);
			void Reset(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement);
			void GetType(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);
			void GetBytes(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);
			void GetDouble(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);
			void GetInt(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);
			void GetInt64(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);
			void GetText(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);
			void GetBlob(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);
			void GetBlob(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,const luabind::object &oAsyncCallback);
			void GetCount(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement);
			void GetColumnName(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);

			void BindDouble(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,double d);
			void BindInt(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,int32_t i);
			void BindInt64(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,int64_t i);
			void BindNull(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);
			void BindText(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,const std::string &text);
			void BindBlob(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,DataStream &ds);
			void ClearBindings(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement);
			void IsReadOnly(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement);
			void GetSQLText(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement);
			void GetExpandedSQLText(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement);
			void GetDatabaseName(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);
			void GetTableName(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);
			void GetOriginName(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);*/
		};
	};
};

extern "C"
{
	void PRAGMA_EXPORT pragma_initialize_lua(Lua::Interface &l)
	{
		if(l.GetIdentifier() == "gui")
			return;
		Lua::sql::register_lua_library(l);
	}
};

/*
int32_t Lua::sql::open(lua_State *l)
{
	std::string fname = Lua::CheckString(l,1);
	if(fname != ":memory:")
	{
		if(add_addon_prefix(l,fname) == false)
			return 0;
	}
	sqlite3 *pDb;
	int32_t r;
	if(Lua::IsSet(l,2) == false)
		r = sqlite3_open(fname.c_str(),&pDb);
	else
	{
		auto flags = Lua::CheckInt(l,2);
		r = sqlite3_open_v2(fname.c_str(),&pDb,flags,nullptr);
	}
	Lua::PushInt(l,r);
	if(r == SQLITE_OK)
	{
		auto con = std::make_shared<::sqlite::Connection>(pDb);
		Lua::Push<std::shared_ptr<::sqlite::Connection>>(l,con);

		CallbackHandle hCb = FunctionCallback<void>::Create(nullptr);
		std::weak_ptr<::sqlite::Connection> wpCon = con;
		hCb.get<Callback<void>>()->SetFunction([hCb,wpCon]() mutable {
			if(wpCon.expired() == false)
				wpCon.lock()->Poll();
			else if(hCb.IsValid())
				hCb.Remove();
		});
		ishared::add_callback(l,ishared::Callback::Think,hCb);
		return 2;
	}
	return 1;
}
int32_t Lua::sql::result_to_string(lua_State *l)
{
	auto errCode = Lua::CheckInt(l,1);
	Lua::PushString(l,::sqlite::Connection::GetResultMessage(errCode));
	return 1;
}*/

/*void Lua::sql::Connection::PrepareStatement(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &statement)
{
	int32_t r;
	auto pStatement = connection->PrepareStatement(statement,r);
	Lua::PushInt(l,r);
	if(r == SQLITE_OK)
		Lua::Push<std::shared_ptr<::sqlite::Statement>>(l,pStatement);
}
void Lua::sql::Connection::PrepareStatement(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &statement,const luabind::object &oAsyncCallback)
{
	Lua::CheckFunction(l,3);
	connection->PrepareStatement(statement,[l,oAsyncCallback](int32_t result,std::shared_ptr<::sqlite::Statement> pStatement) {
		oAsyncCallback.push(l);
		int32_t argCount = 1;
		Lua::PushInt(l,result);
		if(result == SQLITE_OK)
		{
			Lua::Push<std::shared_ptr<::sqlite::Statement>>(l,pStatement);
			++argCount;
		}
		ishared::protected_lua_call(l,argCount,0);
	});
}
void Lua::sql::Connection::OpenBlob(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &tableName,const std::string &columnName,int64_t rowId,bool bReadOnly)
{
	int32_t r;
	auto pBlob = connection->OpenBlob(tableName,columnName,rowId,bReadOnly,r);
	Lua::PushInt(l,r);
	if(r == SQLITE_OK)
		Lua::Push<std::shared_ptr<::sqlite::Blob>>(l,pBlob);
}
void Lua::sql::Connection::OpenBlob(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &tableName,const std::string &columnName,int64_t rowId) {OpenBlob(l,connection,tableName,columnName,rowId,false);}
void Lua::sql::Connection::OpenBlob(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &dbName,const std::string &tableName,const std::string &columnName,int64_t rowId,bool bReadOnly)
{
	int32_t r;
	auto pBlob = connection->OpenBlob(dbName,tableName,columnName,rowId,bReadOnly,r);
	Lua::PushInt(l,r);
	if(r == SQLITE_OK)
		Lua::Push<std::shared_ptr<::sqlite::Blob>>(l,pBlob);
}
void Lua::sql::Connection::OpenBlob(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &dbName,const std::string &tableName,const std::string &columnName,int64_t rowId) {OpenBlob(l,connection,dbName,tableName,columnName,rowId,false);}
void Lua::sql::Connection::Exec(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &sql,const luabind::object &oCallback)
{
	Lua::CheckFunction(l,3);
	std::string errMsg;
	auto r = connection->Exec(sql,[l,&oCallback](int32_t numColumns,char **colStrings,char **colNames) -> int32_t {
		oCallback.push(l);
		Lua::PushInt(l,numColumns);
		auto tColStrings = Lua::CreateTable(l);
		auto tColNames = Lua::CreateTable(l);
		for(auto i=decltype(numColumns){0};i<numColumns;++i)
		{
			Lua::PushInt(l,i +1);
			if(colStrings[i] != nullptr)
				Lua::PushString(l,colStrings[i]);
			else
				Lua::PushNil(l);
			Lua::SetTableValue(l,tColStrings);

			Lua::PushInt(l,i +1);
			Lua::PushString(l,colNames[i]);
			Lua::SetTableValue(l,tColNames);
		}
		if(ishared::protected_lua_call(l,3,1) == true)
		{
			if(Lua::IsBool(l,-1))
			{
				auto b = Lua::CheckBool(l,-1);
				Lua::Pop(l,1);
				return (b == true) ? 0 : 1;
			}
			Lua::Pop(l,1);
		}
		return 0;
	});
	Lua::PushInt(l,r);
	Lua::PushString(l,errMsg);
}
void Lua::sql::Connection::Exec(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &sql)
{
	auto r = connection->Exec(sql,nullptr);
	Lua::PushInt(l,r);
}
void Lua::sql::Connection::GetResult(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection) {Lua::PushInt(l,connection->GetResult());}
void Lua::sql::Connection::GetResultMessage(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection) {Lua::PushString(l,connection->GetResultMessage());}
void Lua::sql::Connection::IsDatabaseReadOnly(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &dbName) {Lua::PushBool(l,connection->IsDatabaseReadOnly(dbName));}
void Lua::sql::Connection::DatabaseExists(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &dbName) {Lua::PushBool(l,connection->DatabaseExists(dbName));}
void Lua::sql::Connection::CreateFunction(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &funcName,int32_t argCount,bool bDeterministic,const luabind::object &oXFunc)
{
	Lua::CheckFunction(l,5);
	auto r = connection->CreateFunction(funcName,argCount,bDeterministic,[l,oXFunc](sqlite3_context *context,int32_t argc,sqlite3_value **argv) {
		oXFunc.push(l);
		Lua::PushInt(l,argc);
		auto t = Lua::CreateTable(l);
		for(auto i=decltype(argc){0};i<argc;++i)
		{
			Lua::PushInt(l,i +1);
			Lua::Push<::sqlite::Value>(l,::sqlite::Value(argv[i]));
			Lua::SetTableValue(l,t);
		}
		ishared::protected_lua_call(l,2,0);
	});
	Lua::PushInt(l,r);
}
void Lua::sql::Connection::CreateFunction(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &funcName,int32_t argCount,bool bDeterministic,const luabind::object &oXStep,const luabind::object &oXFinal)
{
	Lua::CheckFunction(l,5);
	Lua::CheckFunction(l,6);
	auto r = connection->CreateFunction(funcName,argCount,bDeterministic,[l,oXStep](sqlite3_context *context,int32_t argc,sqlite3_value **argv) {
		oXStep.push(l);
		Lua::PushInt(l,argc);
		auto t = Lua::CreateTable(l);
		for(auto i=decltype(argc){0};i<argc;++i)
		{
			Lua::PushInt(l,i +1);
			Lua::Push<::sqlite::Value>(l,::sqlite::Value(argv[i]));
			Lua::SetTableValue(l,t);
		}
		ishared::protected_lua_call(l,2,0);
	},[l,oXFinal](sqlite3_context *context) {
		oXFinal.push(l);
		ishared::protected_lua_call(l,0,0);
	});
	Lua::PushInt(l,r);
}
void Lua::sql::Connection::DeleteFunction(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &funcName,int32_t argCount,bool bDeterministic) {Lua::PushInt(l,connection->DeleteFunction(funcName,argCount,bDeterministic));}

void Lua::sql::Statement::Finalize(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement) {Lua::PushInt(l,statement->Finalize());}
void Lua::sql::Statement::Step(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement) {Lua::PushInt(l,statement->Step());}
void Lua::sql::Statement::Step(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,const luabind::object &oAsynCallback)
{
	Lua::CheckFunction(l,2);
	statement->Step([l,oAsynCallback](int32_t result) {
		oAsynCallback.push(l);
		Lua::PushInt(l,result);
		ishared::protected_lua_call(l,1,0);
	});
}
void Lua::sql::Statement::Reset(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement) {Lua::PushInt(l,statement->Reset());}
void Lua::sql::Statement::GetType(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::PushInt(l,statement->GetType(column));}
void Lua::sql::Statement::GetBytes(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::PushInt(l,statement->GetBytes(column));}
void Lua::sql::Statement::GetDouble(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::PushNumber(l,statement->GetDouble(column));}
void Lua::sql::Statement::GetInt(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::PushInt(l,statement->GetInt(column));}
void Lua::sql::Statement::GetInt64(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::PushInt(l,statement->GetInt64(column));}
void Lua::sql::Statement::GetText(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::PushString(l,statement->GetText(column));}
void Lua::sql::Statement::GetBlob(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::Push<DataStream>(l,statement->GetBlob(column));}
void Lua::sql::Statement::GetBlob(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,const luabind::object &oAsyncCallback)
{
	Lua::CheckFunction(l,3);
	statement->GetBlob(column,[l,oAsyncCallback](DataStream ds) {
		oAsyncCallback.push(l);
		Lua::Push<DataStream>(l,ds);
		ishared::protected_lua_call(l,1,0);
	});
}
void Lua::sql::Statement::GetCount(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement) {Lua::PushInt(l,statement->GetCount());}
void Lua::sql::Statement::GetColumnName(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::PushString(l,statement->GetColumnName(column));}
void Lua::sql::Statement::BindDouble(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,double d) {Lua::PushInt(l,statement->BindDouble(column,d));}
void Lua::sql::Statement::BindInt(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,int32_t i) {Lua::PushInt(l,statement->BindInt(column,i));}
void Lua::sql::Statement::BindInt64(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,int64_t i) {Lua::PushInt(l,statement->BindInt64(column,i));}
void Lua::sql::Statement::BindNull(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::PushInt(l,statement->BindNull(column));}
void Lua::sql::Statement::BindText(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,const std::string &text) {Lua::PushInt(l,statement->BindText(column,text));}
void Lua::sql::Statement::BindBlob(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,DataStream &ds) {Lua::PushInt(l,statement->BindBlob(column,ds));}
void Lua::sql::Statement::ClearBindings(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement) {Lua::PushInt(l,statement->ClearBindings());}
void Lua::sql::Statement::IsReadOnly(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement) {Lua::PushBool(l,statement->IsReadOnly());}
void Lua::sql::Statement::GetSQLText(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement) {Lua::PushString(l,statement->GetSQLText());}
void Lua::sql::Statement::GetExpandedSQLText(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement) {Lua::PushString(l,statement->GetExpandedSQLText());}
void Lua::sql::Statement::GetDatabaseName(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::PushString(l,statement->GetDatabaseName(column));}
void Lua::sql::Statement::GetTableName(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::PushString(l,statement->GetTableName(column));}
void Lua::sql::Statement::GetOriginName(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::PushString(l,statement->GetOriginName(column));}
*/


