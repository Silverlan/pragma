/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

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

		DLLNETWORK void RemoveValue(lua_State*,ds::Block &data,const std::string &key);
		DLLNETWORK void IsEmpty(lua_State *l,ds::Block &data);
		DLLNETWORK void HasValue(lua_State *l,ds::Block &data,const std::string &key);
		DLLNETWORK void AddBlock(lua_State *l,ds::Block &data,const std::string &key);
		DLLNETWORK void FindBlock(lua_State *l,ds::Block &data,const std::string &key);
		DLLNETWORK void FindBlock(lua_State *l,ds::Block &data,const std::string &key,uint32_t blockIndex);
		DLLNETWORK void IsString(lua_State *l,ds::Block &data,const std::string &key);
		DLLNETWORK void IsInt(lua_State *l,ds::Block &data,const std::string &key);
		DLLNETWORK void IsFloat(lua_State *l,ds::Block &data,const std::string &key);
		DLLNETWORK void IsBool(lua_State *l,ds::Block &data,const std::string &key);
		DLLNETWORK void IsColor(lua_State *l,ds::Block &data,const std::string &key);
		DLLNETWORK void IsVector(lua_State *l,ds::Block &data,const std::string &key);
		DLLNETWORK void GetColor(lua_State *l,ds::Block &data,const std::string &key);
		DLLNETWORK void GetVector(lua_State *l,ds::Block &data,const std::string &key);
	};
};

#endif
