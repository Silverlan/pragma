#include "lsql_blob.hpp"
#include <ishared.hpp>

void Lua::sql::Blob::Close(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob) {Lua::PushInt(l,blob->Close());}
void Lua::sql::Blob::GetBytes(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob) {Lua::PushInt(l,blob->GetBytes());}
void Lua::sql::Blob::Read(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob,int32_t size,int32_t offset,const luabind::object &oAsyncCallback)
{
	Lua::CheckFunction(l,4);
	blob->Read(size,offset,[l,oAsyncCallback](::sql::Result result,DataStream ds) {
		oAsyncCallback.push(l);
		int32_t argCount = 1;
		Lua::PushInt(l,umath::to_integral(result));
		if(result == ::sql::Result::Ok)
		{
			Lua::Push<DataStream>(l,ds);
			++argCount;
		}
		ishared::protected_lua_call(l,argCount,0);
	});
}
void Lua::sql::Blob::Read(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob,int32_t size,int32_t offset)
{
	::sql::Result result;
	auto ds = blob->Read(size,offset,result);
	Lua::PushInt(l,umath::to_integral(result));
	if(result != ::sql::Result::Ok)
		return;
	Lua::Push<DataStream>(l,ds);
}
void Lua::sql::Blob::Write(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob,DataStream &ds,int32_t size,int32_t offset,const luabind::object &oAsyncCallback)
{
	Lua::CheckFunction(l,5);
	blob->Write(ds,size,offset,[l,oAsyncCallback](::sql::Result result) {
		oAsyncCallback.push(l);
		Lua::PushInt(l,umath::to_integral(result));
		ishared::protected_lua_call(l,1,0);
	});
}
void Lua::sql::Blob::Write(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob,DataStream &ds,int32_t size,int32_t offset) {Lua::PushInt(l,blob->Write(ds,size,offset));}
void Lua::sql::Blob::Reopen(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob,int64_t rowId) {Lua::PushInt(l,blob->Reopen(rowId));}
