#ifndef __LDATA_HPP__
#define __LDATA_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/networkstate/networkstate.h"
#include "pragma/lua/ldefinitions.h"

namespace ds
{
	class Block;
};
namespace Lua
{
	namespace DataBlock
	{
		DLLNETWORK void GetInt(lua_State *l,ds::Block &data,const std::string &val);
		DLLNETWORK void GetFloat(lua_State *l,ds::Block &data,const std::string &val);
		DLLNETWORK void GetBool(lua_State *l,ds::Block &data,const std::string &val);
		DLLNETWORK void GetString(lua_State *l,ds::Block &data,const std::string &val);
		DLLNETWORK void GetData(lua_State *l,ds::Block &data,const std::string &val);
		DLLNETWORK void SetValue(lua_State *l,ds::Block &data,const std::string &type,const std::string &key,const std::string &val);
	};
};

#endif
