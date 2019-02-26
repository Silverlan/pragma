#include "lsql_statement.hpp"
#include "sql_statement.hpp"
#include <ishared.hpp>

void Lua::sql::Statement::Step(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement) {statement->Step([](::sql::Result asResult) {});}
void Lua::sql::Statement::Step(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,const luabind::object &oCallback)
{
	auto bNil = !Lua::IsSet(l,2);
	if(bNil == false)
		Lua::CheckFunction(l,2);
	statement->Step([l,oCallback,bNil](::sql::Result result) {
		if(bNil == true)
			return;
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		if(ishared::protected_lua_call(l,1,0) == true)
			;
	});
}
void Lua::sql::Statement::ExecuteQuery(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement) {statement->ExecuteQuery([](::sql::Result asResult) {});}
void Lua::sql::Statement::ExecuteQuery(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,const luabind::object &oCallback)
{
	auto bNil = !Lua::IsSet(l,2);
	if(bNil == false)
		Lua::CheckFunction(l,2);
	statement->ExecuteQuery([l,oCallback,bNil](::sql::Result result) {
		if(bNil == true)
			return;
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		if(ishared::protected_lua_call(l,1,0) == true)
			;
	});
}
void Lua::sql::Statement::Finalize(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement) {statement->Finalize([](::sql::Result asResult) {});}
void Lua::sql::Statement::Finalize(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,const luabind::object &oCallback)
{
	auto bNil = !Lua::IsSet(l,2);
	if(bNil == false)
		Lua::CheckFunction(l,2);
	statement->Finalize([l,oCallback,bNil](::sql::Result result) {
		if(bNil == true)
			return;
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		if(ishared::protected_lua_call(l,1,0) == true)
			;
	});
}
void Lua::sql::Statement::Reset(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement) {statement->Reset([](::sql::Result asResult) {});}
void Lua::sql::Statement::Reset(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,const luabind::object &oCallback)
{
	auto bNil = !Lua::IsSet(l,2);
	if(bNil == false)
		Lua::CheckFunction(l,2);
	statement->Reset([l,oCallback,bNil](::sql::Result result) {
		if(bNil == true)
			return;
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		if(ishared::protected_lua_call(l,1,0) == true)
			;
	});
}
void Lua::sql::Statement::FindColumn(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,const std::string &colName)
{
	auto result = ::sql::Result::Ok;
	int32_t colId = -1;
	statement->FindColumn(colName,[&result,&colId](::sql::Result asResult,uint32_t asColId) {result = asResult; colId = asColId;});
	statement->Wait();
	Lua::PushInt(l,umath::to_integral(result));
	if(result == ::sql::Result::Ok)
		Lua::PushInt(l,colId);
}
void Lua::sql::Statement::FindColumn(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,const std::string &colName,const luabind::object &oCallback)
{
	auto bNil = !Lua::IsSet(l,3);
	if(bNil == false)
		Lua::CheckFunction(l,3);
	statement->FindColumn(colName,[l,oCallback,bNil](::sql::Result result,uint32_t colId) {
		if(bNil == true)
			return;
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		int32_t numArgs = 1;
		if(result == ::sql::Result::Ok)
		{
			Lua::PushInt(l,colId);
			++numArgs;
		}
		if(ishared::protected_lua_call(l,numArgs,0) == true)
			;
	});
}

void Lua::sql::Statement::IsReadOnly(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column)
{
	auto result = ::sql::Result::Ok;
	auto bReadOnly = false;
	statement->IsReadOnly(column,[&result,&bReadOnly](::sql::Result asResult,bool asReadOnly) {result = asResult; bReadOnly = asReadOnly;});
	statement->Wait();
	Lua::PushInt(l,umath::to_integral(result));
	if(result == ::sql::Result::Ok)
		Lua::PushBool(l,bReadOnly);
}
void Lua::sql::Statement::IsReadOnly(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column,const luabind::object &oCallback)
{
	auto bNil = !Lua::IsSet(l,3);
	if(bNil == false)
		Lua::CheckFunction(l,3);
	statement->IsReadOnly(column,[l,oCallback,bNil](::sql::Result result,bool bReadOnly) {
		if(bNil == true)
			return;
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		int32_t numArgs = 1;
		if(result == ::sql::Result::Ok)
		{
			Lua::PushBool(l,bReadOnly);
			++numArgs;
		}
		if(ishared::protected_lua_call(l,numArgs,0) == true)
			;
	});
}
void Lua::sql::Statement::GetType(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column)
{
	auto result = ::sql::Result::Ok;
	auto type = ::sql::Type::Initial;
	statement->GetType(column,[&result,&type](::sql::Result asResult,::sql::Type asType) {result = asResult; type = asType;});
	statement->Wait();
	Lua::PushInt(l,umath::to_integral(result));
	if(result == ::sql::Result::Ok)
		Lua::PushInt(l,umath::to_integral(type));
}
void Lua::sql::Statement::GetType(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column,const luabind::object &oCallback)
{
	auto bNil = !Lua::IsSet(l,3);
	if(bNil == false)
		Lua::CheckFunction(l,3);
	statement->GetType(column,[l,oCallback,bNil](::sql::Result result,::sql::Type type) {
		if(bNil == true)
			return;
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		int32_t numArgs = 1;
		if(result == ::sql::Result::Ok)
		{
			Lua::PushInt(l,umath::to_integral(type));
			++numArgs;
		}
		if(ishared::protected_lua_call(l,numArgs,0) == true)
			;
	});
}
void Lua::sql::Statement::GetDouble(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column)
{
	auto result = ::sql::Result::Ok;
	auto val = 0.0;
	statement->GetDouble(column,[&result,&val](::sql::Result asResult,double asV) {result = asResult; val = asV;});
	statement->Wait();
	Lua::PushInt(l,umath::to_integral(result));
	if(result == ::sql::Result::Ok)
		Lua::PushNumber(l,val);
}
void Lua::sql::Statement::GetDouble(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column,const luabind::object &oCallback)
{
	auto bNil = !Lua::IsSet(l,3);
	if(bNil == false)
		Lua::CheckFunction(l,3);
	statement->GetDouble(column,[l,oCallback,bNil](::sql::Result result,double v) {
		if(bNil == true)
			return;
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		int32_t numArgs = 1;
		if(result == ::sql::Result::Ok)
		{
			Lua::PushNumber(l,v);
			++numArgs;
		}
		if(ishared::protected_lua_call(l,numArgs,0) == true)
			;
	});
}
void Lua::sql::Statement::GetInt(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column)
{
	auto result = ::sql::Result::Ok;
	auto val = 0;
	statement->GetInt(column,[&result,&val](::sql::Result asResult,int32_t asV) {result = asResult; val = asV;});
	statement->Wait();
	Lua::PushInt(l,umath::to_integral(result));
	if(result == ::sql::Result::Ok)
		Lua::PushInt(l,val);
}
void Lua::sql::Statement::GetInt(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column,const luabind::object &oCallback)
{
	auto bNil = !Lua::IsSet(l,3);
	if(bNil == false)
		Lua::CheckFunction(l,3);
	statement->GetInt(column,[l,oCallback,bNil](::sql::Result result,int32_t v) {
		if(bNil == true)
			return;
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		int32_t numArgs = 1;
		if(result == ::sql::Result::Ok)
		{
			Lua::PushInt(l,v);
			++numArgs;
		}
		if(ishared::protected_lua_call(l,numArgs,0) == true)
			;
	});
}
void Lua::sql::Statement::GetInt64(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column)
{
	auto result = ::sql::Result::Ok;
	auto val = 0ll;
	statement->GetInt64(column,[&result,&val](::sql::Result asResult,int64_t asV) {result = asResult; val = asV;});
	statement->Wait();
	Lua::PushInt(l,umath::to_integral(result));
	if(result == ::sql::Result::Ok)
		Lua::PushInt(l,val);
}
void Lua::sql::Statement::GetInt64(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column,const luabind::object &oCallback)
{
	auto bNil = !Lua::IsSet(l,3);
	if(bNil == false)
		Lua::CheckFunction(l,3);
	statement->GetInt64(column,[l,oCallback,bNil](::sql::Result result,int64_t v) {
		if(bNil == true)
			return;
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		int32_t numArgs = 1;
		if(result == ::sql::Result::Ok)
		{
			Lua::PushInt(l,v);
			++numArgs;
		}
		if(ishared::protected_lua_call(l,numArgs,0) == true)
			;
	});
}
void Lua::sql::Statement::GetText(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column)
{
	auto result = ::sql::Result::Ok;
	std::string val {};
	statement->GetText(column,[&result,&val](::sql::Result asResult,const std::string &asV) {result = asResult; val = asV;});
	statement->Wait();
	Lua::PushInt(l,umath::to_integral(result));
	if(result == ::sql::Result::Ok)
		Lua::PushString(l,val);
}
void Lua::sql::Statement::GetText(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column,const luabind::object &oCallback)
{
	auto bNil = !Lua::IsSet(l,3);
	if(bNil == false)
		Lua::CheckFunction(l,3);
	statement->GetText(column,[l,oCallback,bNil](::sql::Result result,const std::string &v) {
		if(bNil == true)
			return;
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		int32_t numArgs = 1;
		if(result == ::sql::Result::Ok)
		{
			Lua::PushString(l,v);
			++numArgs;
		}
		if(ishared::protected_lua_call(l,numArgs,0) == true)
			;
	});
}
void Lua::sql::Statement::GetBlob(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column)
{
	auto result = ::sql::Result::Ok;
	DataStream val {};
	statement->GetBlob(column,[&result,&val](::sql::Result asResult,const DataStream &asV) {result = asResult; val = asV;});
	statement->Wait();
	Lua::PushInt(l,umath::to_integral(result));
	if(result == ::sql::Result::Ok)
		Lua::Push<DataStream>(l,val);
}
void Lua::sql::Statement::GetBlob(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column,const luabind::object &oCallback)
{
	auto bNil = !Lua::IsSet(l,3);
	if(bNil == false)
		Lua::CheckFunction(l,3);
	statement->GetBlob(column,[l,oCallback,bNil](::sql::Result result,const DataStream &v) {
		if(bNil == true)
			return;
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		int32_t numArgs = 1;
		if(result == ::sql::Result::Ok)
		{
			Lua::Push<DataStream>(l,v);
			++numArgs;
		}
		if(ishared::protected_lua_call(l,numArgs,0) == true)
			;
	});
}
void Lua::sql::Statement::GetColumnCount(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement)
{
	auto result = ::sql::Result::Ok;
	int32_t val {};
	statement->GetColumnCount([&result,&val](::sql::Result asResult,int32_t asVal) {result = asResult; val = asVal;});
	statement->Wait();
	Lua::PushInt(l,umath::to_integral(result));
	if(result == ::sql::Result::Ok)
		Lua::PushInt(l,val);
}
void Lua::sql::Statement::GetColumnCount(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,const luabind::object &oCallback)
{
	auto bNil = !Lua::IsSet(l,2);
	if(bNil == false)
		Lua::CheckFunction(l,2);
	statement->GetColumnCount([l,oCallback,bNil](::sql::Result result,int32_t v) {
		if(bNil == true)
			return;
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		int32_t numArgs = 1;
		if(result == ::sql::Result::Ok)
		{
			Lua::PushInt(l,v);
			++numArgs;
		}
		if(ishared::protected_lua_call(l,numArgs,0) == true)
			;
	});
}
void Lua::sql::Statement::GetColumnName(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column)
{
	auto result = ::sql::Result::Ok;
	std::string val {};
	statement->GetColumnName(column,[&result,&val](::sql::Result asResult,const std::string &asVal) {result = asResult; val = asVal;});
	statement->Wait();
	Lua::PushInt(l,umath::to_integral(result));
	if(result == ::sql::Result::Ok)
		Lua::PushString(l,val);
}
void Lua::sql::Statement::GetColumnName(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column,const luabind::object &oCallback)
{
	auto bNil = !Lua::IsSet(l,3);
	if(bNil == false)
		Lua::CheckFunction(l,3);
	statement->GetColumnName(column,[l,oCallback,bNil](::sql::Result result,const std::string &v) {
		if(bNil == true)
			return;
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		int32_t numArgs = 1;
		if(result == ::sql::Result::Ok)
		{
			Lua::PushString(l,v);
			++numArgs;
		}
		if(ishared::protected_lua_call(l,numArgs,0) == true)
			;
	});
}
void Lua::sql::Statement::GetRowCount(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement)
{
	auto result = ::sql::Result::Ok;
	size_t val {};
	statement->GetRowCount([&result,&val](::sql::Result asResult,size_t asVal) {result = asResult; val = asVal;});
	statement->Wait();
	Lua::PushInt(l,umath::to_integral(result));
	if(result == ::sql::Result::Ok)
		Lua::PushInt(l,val);
}
void Lua::sql::Statement::GetRowCount(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,const luabind::object &oCallback)
{
	auto bNil = !Lua::IsSet(l,2);
	if(bNil == false)
		Lua::CheckFunction(l,2);
	statement->GetRowCount([l,oCallback,bNil](::sql::Result result,size_t v) {
		if(bNil == true)
			return;
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		int32_t numArgs = 1;
		if(result == ::sql::Result::Ok)
		{
			Lua::PushInt(l,v);
			++numArgs;
		}
		if(ishared::protected_lua_call(l,numArgs,0) == true)
			;
	});
}
void Lua::sql::Statement::GetValue(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column)
{
	// TODO
}
void Lua::sql::Statement::GetValue(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,int32_t column,const luabind::object &oCallback)
{
	auto bNil = !Lua::IsSet(l,3);
	if(bNil == false)
		Lua::CheckFunction(l,3);
	/*statement->GetValue([l,oCallback,bNil](::sql::Result result,size_t v) {
		if(bNil == true)
			return;
		oCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		int32_t numArgs = 1;
		if(result == ::sql::Result::Ok)
		{
			Lua::PushInt(l,v);
			++numArgs;
		}
		if(ishared::protected_lua_call(l,numArgs,0) == true)
			;
	});*/
}
void Lua::sql::Statement::ExecuteQueryAndFetchRows(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,const std::string &sql)
{

}
void Lua::sql::Statement::ExecuteQueryAndFetchRows(lua_State *l,std::shared_ptr<::sql::BaseStatement> &statement,const std::string &sql,const luabind::object &oCallback)
{
	/*std::vector<std::vector<::sql::PValue>> rows {};
	auto r = statement->ExecuteQueryAndFetchRows(sql,rows);
	Lua::PushInt(l,umath::to_integral(r));
	if(r == ::sql::Result::Ok)
	{
		auto t = Lua::CreateTable(l);
		auto rowIdx = 1u;
		for(auto &row : rows)
		{
			Lua::PushInt(l,rowIdx++);
			auto tValues = Lua::CreateTable(l);
			auto colIdx = 1u;
			for(auto &val : row)
			{
				Lua::PushInt(l,colIdx++);
				Lua::Push<::sql::PValue>(l,val);
				Lua::SetTableValue(l,tValues);
			}
			Lua::SetTableValue(l,t);
		}
	}*/
}

