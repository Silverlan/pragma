#ifndef __LSQL_BLOB_HPP__
#define __LSQL_BLOB_HPP__

#include <memory>
#include <sharedutils/datastream.h>
#include <luasystem.h>
#include "sqlite_blob.hpp"

struct lua_State;
namespace luabind {class object;};

namespace Lua
{
	namespace sql
	{
		namespace Blob
		{
			void Close(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob);
			void GetBytes(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob);
			void Read(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob,int32_t size,int32_t offset);
			void Read(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob,int32_t size,int32_t offset,const luabind::object &oAsyncCallback);
			void Write(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob,DataStream &ds,int32_t size,int32_t offset);
			void Write(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob,DataStream &ds,int32_t size,int32_t offset,const luabind::object &oAsyncCallback);
			void Reopen(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob,int64_t rowId);
		};
	};
};

#endif
