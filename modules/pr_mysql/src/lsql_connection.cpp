#include "lsql_connection.hpp"
#include "lsql_value.hpp"
#include "sql_connection.hpp"
#include <mathutil/umath.h>
#include <ishared.hpp>

static void push_rows(lua_State *l,std::vector<std::vector<::sql::PValue>> &rows)
{
	auto t = Lua::CreateTable(l);
	int32_t rowId = 1;
	for(auto &row : rows)
	{
		Lua::PushInt(l,rowId++);
		auto tRow = Lua::CreateTable(l);
		int32_t valId = 1;
		for(auto &v : row)
		{
			Lua::PushInt(l,valId++);
			Lua::Push<::sql::PValue>(l,v);
			Lua::SetTableValue(l,tRow);
		}
		Lua::SetTableValue(l,t);
	}
}
static void push_columns(lua_State *l,std::vector<std::string> &columnNames)
{
	auto t = Lua::CreateTable(l);
	int32_t colId = 1;
	for(auto &colName : columnNames)
	{
		Lua::PushInt(l,colId++);
		Lua::PushString(l,colName);
		Lua::SetTableValue(l,t);
	}
}
static std::shared_ptr<std::vector<::sql::PValue>> get_values(lua_State *l,int32_t tValues)
{
	auto values = std::make_shared<std::vector<::sql::PValue>>();
	auto numValues = Lua::GetObjectLength(l,tValues);
	values->reserve(numValues);
	for(auto i=decltype(numValues){0};i<numValues;++i)
	{
		Lua::PushInt(l,i +1);
		Lua::GetTableValue(l,tValues);

		auto *val = Lua::CheckSQLValue(l,-1);
		values->push_back(*val);

		Lua::Pop(l,1);
	}
	return values;
}
void Lua::sql::Connection::ExecuteQueryAndFetchRows(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const std::string &sql)
{
	auto result = ::sql::Result::Ok;
	std::shared_ptr<::sql::QueryData> queryData = nullptr;
	connection->ExecuteQueryAndFetchRows(sql,[&result,&queryData](::sql::Result asResult,std::shared_ptr<::sql::QueryData> &asQueryData) {result = asResult; queryData = asQueryData;});
	connection->Wait();
	Lua::PushInt(l,umath::to_integral(result));
	if(result == ::sql::Result::Ok)
	{
		push_columns(l,queryData->columnNames);
		push_rows(l,queryData->rows);
	}
}
void Lua::sql::Connection::ExecuteQueryAndFetchRows(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const std::string &sql,const luabind::object &oCallback)
{
	if(Lua::IsFunction(l,3) == false)
	{
		auto tValues = 3;
		Lua::CheckTable(l,tValues);
		auto result = ::sql::Result::Ok;
		auto values = get_values(l,tValues);
		std::shared_ptr<::sql::QueryData> queryData = nullptr;
		connection->ExecuteQueryAndFetchRows(sql,values,[&result,&queryData](::sql::Result asResult,std::shared_ptr<::sql::QueryData> &asQueryData) {result = asResult; queryData = asQueryData;});
		connection->Wait();
		Lua::PushInt(l,umath::to_integral(result));
		if(result == ::sql::Result::Ok)
		{
			push_columns(l,queryData->columnNames);
			push_rows(l,queryData->rows);
		}
		return;
	}
	Lua::CheckFunction(l,3);
	connection->ExecuteQueryAndFetchRows(sql,[l,oCallback](::sql::Result result,std::shared_ptr<::sql::QueryData> &queryData) {
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		int32_t numArgs = 1;
		if(result == ::sql::Result::Ok)
		{
			numArgs += 2;
			push_columns(l,queryData->columnNames);
			push_rows(l,queryData->rows);
		}
		if(ishared::protected_lua_call(l,numArgs,0) == true)
			;
	});
}
void Lua::sql::Connection::ExecuteQueryAndFetchRows(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const std::string &sql,const luabind::object &oValues,const luabind::object &oCallback)
{
	Lua::CheckTable(l,3);
	Lua::CheckFunction(l,4);
	auto values = get_values(l,3);
	connection->ExecuteQueryAndFetchRows(sql,values,[l,oCallback](::sql::Result result,std::shared_ptr<::sql::QueryData> &queryData) {
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		int32_t numArgs = 1;
		if(result == ::sql::Result::Ok)
		{
			numArgs += 2;
			push_columns(l,queryData->columnNames);
			push_rows(l,queryData->rows);
		}
		if(ishared::protected_lua_call(l,numArgs,0) == true)
			;
	});
}
void Lua::sql::Connection::Execute(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const std::string &sql,const luabind::object &oValues,const luabind::object &oCallback)
{
	auto tValues = 3;
	Lua::CheckTable(l,tValues);
	Lua::CheckFunction(l,4);
	auto result = ::sql::Result::Ok;
	auto values = get_values(l,tValues);
	connection->Execute(sql,values,[l,oCallback](::sql::Result result) {
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		if(ishared::protected_lua_call(l,1,0) == true)
			;
	});
}
void Lua::sql::Connection::Execute(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const std::string &sql,const luabind::object &oCallback)
{
	if(Lua::IsFunction(l,3) == false)
	{
		auto tValues = 3;
		Lua::CheckTable(l,tValues);
		auto result = ::sql::Result::Ok;
		auto values = get_values(l,tValues);
		connection->Execute(sql,[&result](::sql::Result asResult) {result = asResult;});
		connection->Wait();
		Lua::PushInt(l,umath::to_integral(result));
		return;
	}
	connection->Execute(sql,[l,oCallback](::sql::Result result) {
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		if(ishared::protected_lua_call(l,1,0) == true)
			;
	});
}
void Lua::sql::Connection::Execute(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const std::string &sql)
{
	connection->Execute(sql,[](::sql::Result asResult) {});
}
void Lua::sql::Connection::Close(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection)
{
	auto r = connection->Close();
	Lua::PushInt(l,umath::to_integral(r));
}
void Lua::sql::Connection::GetLastInsertRowId(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection)
{
	auto result = ::sql::Result::Ok;
	int64_t rowId = -1;
	connection->GetLastInsertRowId([&result,&rowId](::sql::Result asResult,int64_t asRowId) {result = asResult; rowId = asRowId;});
	connection->Wait();
	Lua::PushInt(l,umath::to_integral(result));
	if(result == ::sql::Result::Ok)
		Lua::PushInt(l,rowId);
}
void Lua::sql::Connection::GetLastInsertRowId(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const luabind::object &oCallback)
{
	auto bNil = !Lua::IsSet(l,2);
	if(bNil == false)
		Lua::CheckFunction(l,2);
	connection->GetLastInsertRowId([l,oCallback,bNil](::sql::Result result,int64_t rowId) {
		if(bNil == true)
			return;
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		auto numArgs = 1;
		if(result == ::sql::Result::Ok)
		{
			Lua::PushInt(l,rowId);
			++numArgs;
		}
		if(ishared::protected_lua_call(l,numArgs,0) == true)
			;
	});
}
void Lua::sql::Connection::CreateStatement(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const std::string &sql)
{
	auto result = ::sql::Result::Ok;
	std::shared_ptr<::sql::BaseStatement> statement = nullptr;
	connection->CreateStatement(sql,[&result,&statement](::sql::Result asResult,std::shared_ptr<::sql::BaseStatement> asStatement) {
		result = asResult;
		statement = asStatement;
	});
	connection->Wait();
	Lua::PushInt(l,umath::to_integral(result));
	if(result == ::sql::Result::Ok)
		Lua::Push<std::shared_ptr<::sql::BaseStatement>>(l,statement);
}
void Lua::sql::Connection::CreateStatement(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const std::string &sql,const luabind::object &oCallback)
{
	auto bNil = !Lua::IsSet(l,3);
	if(bNil == false)
		Lua::CheckFunction(l,3);
	connection->CreateStatement(sql,[l,oCallback,bNil](::sql::Result result,std::shared_ptr<::sql::BaseStatement> statement) {
		if(bNil == true)
			return;
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		int32_t numArgs = 1;
		if(result == ::sql::Result::Ok)
		{
			Lua::Push<std::shared_ptr<::sql::BaseStatement>>(l,statement);
			++numArgs;
		}
		if(ishared::protected_lua_call(l,numArgs,0) == true)
			;
	});
}
void Lua::sql::Connection::CreatePreparedStatement(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const std::string &sql)
{
	auto result = ::sql::Result::Ok;
	std::shared_ptr<::sql::BasePreparedStatement> statement = nullptr;
	connection->CreatePreparedStatement(sql,[&result,&statement](::sql::Result asResult,std::shared_ptr<::sql::BasePreparedStatement> asStatement) {
		result = asResult;
		statement = asStatement;
	});
	connection->Wait();
	Lua::PushInt(l,umath::to_integral(result));
	if(result == ::sql::Result::Ok)
		Lua::Push<std::shared_ptr<::sql::BasePreparedStatement>>(l,statement);
}
void Lua::sql::Connection::CreatePreparedStatement(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const std::string &sql,const luabind::object &oCallback)
{
	auto bNil = !Lua::IsSet(l,3);
	if(bNil == false)
		Lua::CheckFunction(l,3);
	connection->CreatePreparedStatement(sql,[l,oCallback,bNil](::sql::Result result,std::shared_ptr<::sql::BasePreparedStatement> statement) {
		if(bNil == true)
			return;
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		int32_t numArgs = 1;
		if(result == ::sql::Result::Ok)
		{
			Lua::Push<std::shared_ptr<::sql::BasePreparedStatement>>(l,statement);
			++numArgs;
		}
		if(ishared::protected_lua_call(l,numArgs,0) == true)
			;
	});
}
void Lua::sql::Connection::Wait(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection) {connection->Wait();}
void Lua::sql::Connection::CallOnQueueComplete(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection,const luabind::object &oCallback)
{
	Lua::CheckFunction(l,2);
	connection->CallOnQueueComplete([l,oCallback]() {
		oCallback.push(l);
		if(ishared::protected_lua_call(l,0,0) == true)
			;
	});
}
void Lua::sql::Connection::GetResultCode(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection)
{
	Lua::PushInt(l,umath::to_integral(connection->GetResultCode()));
}
void Lua::sql::Connection::GetResultErrorMessage(lua_State *l,std::shared_ptr<::sql::BaseConnection> &connection)
{
	Lua::PushString(l,connection->GetResultErrorMessage());
}
