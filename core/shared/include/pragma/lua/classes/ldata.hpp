/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LDATA_HPP__
#define __LDATA_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/networkstate/networkstate.h"
#include "pragma/lua/ldefinitions.h"

namespace ds {
	class Block;
};
class LFile;
namespace Lua {
	namespace DataBlock {
		DLLNETWORK void load(lua_State *l, const std::string &fileName);
		DLLNETWORK void load(lua_State *l, LFile &f);
		DLLNETWORK void create(lua_State *l);

		DLLNETWORK void GetInt(lua_State *l, ds::Block &data, const std::string &val);
		DLLNETWORK void GetFloat(lua_State *l, ds::Block &data, const std::string &val);
		DLLNETWORK void GetBool(lua_State *l, ds::Block &data, const std::string &val);
		DLLNETWORK void GetString(lua_State *l, ds::Block &data, const std::string &val);
		DLLNETWORK void GetColor(lua_State *l, ds::Block &data, const std::string &key);
		DLLNETWORK void GetVector(lua_State *l, ds::Block &data, const std::string &key);
		DLLNETWORK void GetVector2(lua_State *l, ds::Block &data, const std::string &key);
		DLLNETWORK void GetVector4(lua_State *l, ds::Block &data, const std::string &key);
		DLLNETWORK void GetValue(lua_State *l, ds::Block &data, const std::string &key);

		DLLNETWORK void GetInt(lua_State *l, ds::Block &data, const std::string &val, int32_t def);
		DLLNETWORK void GetFloat(lua_State *l, ds::Block &data, const std::string &val, float def);
		DLLNETWORK void GetBool(lua_State *l, ds::Block &data, const std::string &val, bool def);
		DLLNETWORK void GetString(lua_State *l, ds::Block &data, const std::string &val, const std::string &def);
		DLLNETWORK void GetColor(lua_State *l, ds::Block &data, const std::string &key, const Color &def);
		DLLNETWORK void GetVector(lua_State *l, ds::Block &data, const std::string &key, const Vector3 &def);
		DLLNETWORK void GetVector2(lua_State *l, ds::Block &data, const std::string &key, const ::Vector2 &def);
		DLLNETWORK void GetVector4(lua_State *l, ds::Block &data, const std::string &key, const ::Vector4 &def);

		DLLNETWORK void GetData(lua_State *l, ds::Block &data, const std::string &val);
		DLLNETWORK void GetChildBlocks(lua_State *l, ds::Block &data);
		DLLNETWORK void SetValue(lua_State *l, ds::Block &data, const std::string &type, const std::string &key, const std::string &val);
		DLLNETWORK void Merge(lua_State *l, ds::Block &data, ds::Block &other);

		DLLNETWORK void RemoveValue(lua_State *, ds::Block &data, const std::string &key);
		DLLNETWORK void IsEmpty(lua_State *l, ds::Block &data);
		DLLNETWORK void HasValue(lua_State *l, ds::Block &data, const std::string &key);
		DLLNETWORK void AddBlock(lua_State *l, ds::Block &data, const std::string &key);
		DLLNETWORK void FindBlock(lua_State *l, ds::Block &data, const std::string &key);
		DLLNETWORK void FindBlock(lua_State *l, ds::Block &data, const std::string &key, uint32_t blockIndex);
		DLLNETWORK void IsString(lua_State *l, ds::Block &data, const std::string &key);
		DLLNETWORK void IsInt(lua_State *l, ds::Block &data, const std::string &key);
		DLLNETWORK void IsFloat(lua_State *l, ds::Block &data, const std::string &key);
		DLLNETWORK void IsBool(lua_State *l, ds::Block &data, const std::string &key);
		DLLNETWORK void IsColor(lua_State *l, ds::Block &data, const std::string &key);
		DLLNETWORK void IsVector(lua_State *l, ds::Block &data, const std::string &key);
		DLLNETWORK void IsVector4(lua_State *l, ds::Block &data, const std::string &key);

		DLLNETWORK void ToString(lua_State *l, ds::Block &data, const std::string &rootIdentifier, uint8_t tabDepth);
		DLLNETWORK void ToString(lua_State *l, ds::Block &data, const std::string &rootIdentifier);
		DLLNETWORK void ToString(lua_State *l, ds::Block &data, uint8_t tabDepth);
		DLLNETWORK void ToString(lua_State *l, ds::Block &data);
	};
};

#endif
