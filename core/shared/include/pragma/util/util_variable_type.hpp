/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __UTIL_VARIABLE_TYPE_HPP__
#define __UTIL_VARIABLE_TYPE_HPP__

#include "pragma/networkdefinitions.h"
#include <sharedutils/netpacket.hpp>
#include <any>

namespace util {
	enum class VarType : uint8_t {
		Invalid = 0,
		Bool,
		Double,
		Float,
		Int8,
		Int16,
		Int32,
		Int64,
		LongDouble,
		String,
		UInt8,
		UInt16,
		UInt32,
		UInt64,
		EulerAngles,
		Color,
		Vector,
		Vector2,
		Vector4,
		Entity,
		Quaternion,
		Transform,
		ScaledTransform,

		Count
	};
	std::string variable_type_to_string(VarType type);
};

struct lua_State;
class DataStream;
class Game;
namespace Lua {
	DLLNETWORK std::any GetAnyValue(lua_State *l, ::util::VarType varType, int32_t idx);
	DLLNETWORK std::any GetAnyPropertyValue(lua_State *l, int32_t indexProperty, ::util::VarType varType);
	DLLNETWORK void SetAnyPropertyValue(lua_State *l, int32_t indexProperty, ::util::VarType varType, const std::any &value);
	DLLNETWORK void PushAny(lua_State *l, ::util::VarType varType, const std::any &value);
	DLLNETWORK void PushNewAnyProperty(lua_State *l, ::util::VarType varType, const std::any &value);

	DLLNETWORK void WriteAny(::DataStream &ds, ::util::VarType varType, const std::any &value, uint32_t *pos = nullptr);
	DLLNETWORK void WriteAny(::NetPacket &ds, ::util::VarType varType, const std::any &value, uint32_t *pos = nullptr);
	DLLNETWORK void ReadAny(Game &game, ::DataStream &ds, ::util::VarType varType, std::any &outValue);
	DLLNETWORK void ReadAny(::NetPacket &ds, ::util::VarType varType, std::any &outValue);
};

#endif
