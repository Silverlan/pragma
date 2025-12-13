// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.data_block;

import :scripting.lua.libraries.file;
export import pragma.datasystem;

export namespace Lua {
	namespace DataBlock {
		DLLNETWORK void load(lua::State *l, const std::string &fileName);
		DLLNETWORK void load(lua::State *l, LFile &f);
		DLLNETWORK void create(lua::State *l);

		DLLNETWORK void GetInt(lua::State *l, pragma::datasystem::Block &data, const std::string &val);
		DLLNETWORK void GetFloat(lua::State *l, pragma::datasystem::Block &data, const std::string &val);
		DLLNETWORK void GetBool(lua::State *l, pragma::datasystem::Block &data, const std::string &val);
		DLLNETWORK void GetString(lua::State *l, pragma::datasystem::Block &data, const std::string &val);
		DLLNETWORK void GetColor(lua::State *l, pragma::datasystem::Block &data, const std::string &key);
		DLLNETWORK void GetVector(lua::State *l, pragma::datasystem::Block &data, const std::string &key);
		DLLNETWORK void GetVector2(lua::State *l, pragma::datasystem::Block &data, const std::string &key);
		DLLNETWORK void GetVector4(lua::State *l, pragma::datasystem::Block &data, const std::string &key);
		DLLNETWORK void GetValue(lua::State *l, pragma::datasystem::Block &data, const std::string &key);

		DLLNETWORK void GetInt(lua::State *l, pragma::datasystem::Block &data, const std::string &val, int32_t def);
		DLLNETWORK void GetFloat(lua::State *l, pragma::datasystem::Block &data, const std::string &val, float def);
		DLLNETWORK void GetBool(lua::State *l, pragma::datasystem::Block &data, const std::string &val, bool def);
		DLLNETWORK void GetString(lua::State *l, pragma::datasystem::Block &data, const std::string &val, const std::string &def);
		DLLNETWORK void GetColor(lua::State *l, pragma::datasystem::Block &data, const std::string &key, const Color &def);
		DLLNETWORK void GetVector(lua::State *l, pragma::datasystem::Block &data, const std::string &key, const Vector3 &def);
		DLLNETWORK void GetVector2(lua::State *l, pragma::datasystem::Block &data, const std::string &key, const Vector2 &def);
		DLLNETWORK void GetVector4(lua::State *l, pragma::datasystem::Block &data, const std::string &key, const Vector4 &def);

		DLLNETWORK void GetData(lua::State *l, pragma::datasystem::Block &data, const std::string &val);
		DLLNETWORK void GetChildBlocks(lua::State *l, pragma::datasystem::Block &data);
		DLLNETWORK void SetValue(lua::State *l, pragma::datasystem::Block &data, const std::string &type, const std::string &key, const std::string &val);
		DLLNETWORK void Merge(lua::State *l, pragma::datasystem::Block &data, pragma::datasystem::Block &other);

		DLLNETWORK void RemoveValue(lua::State *, pragma::datasystem::Block &data, const std::string &key);
		DLLNETWORK void IsEmpty(lua::State *l, pragma::datasystem::Block &data);
		DLLNETWORK void HasValue(lua::State *l, pragma::datasystem::Block &data, const std::string &key);
		DLLNETWORK void AddBlock(lua::State *l, pragma::datasystem::Block &data, const std::string &key);
		DLLNETWORK void FindBlock(lua::State *l, pragma::datasystem::Block &data, const std::string &key);
		DLLNETWORK void FindBlock(lua::State *l, pragma::datasystem::Block &data, const std::string &key, uint32_t blockIndex);
		DLLNETWORK void IsString(lua::State *l, pragma::datasystem::Block &data, const std::string &key);
		DLLNETWORK void IsInt(lua::State *l, pragma::datasystem::Block &data, const std::string &key);
		DLLNETWORK void IsFloat(lua::State *l, pragma::datasystem::Block &data, const std::string &key);
		DLLNETWORK void IsBool(lua::State *l, pragma::datasystem::Block &data, const std::string &key);
		DLLNETWORK void IsColor(lua::State *l, pragma::datasystem::Block &data, const std::string &key);
		DLLNETWORK void IsVector(lua::State *l, pragma::datasystem::Block &data, const std::string &key);
		DLLNETWORK void IsVector4(lua::State *l, pragma::datasystem::Block &data, const std::string &key);

		DLLNETWORK void ToString(lua::State *l, pragma::datasystem::Block &data, const std::string &rootIdentifier, uint8_t tabDepth);
		DLLNETWORK void ToString(lua::State *l, pragma::datasystem::Block &data, const std::string &rootIdentifier);
		DLLNETWORK void ToString(lua::State *l, pragma::datasystem::Block &data, uint8_t tabDepth);
		DLLNETWORK void ToString(lua::State *l, pragma::datasystem::Block &data);
	};
};
