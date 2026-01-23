// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include <any>

export module pragma.shared:util.any;

export import :types;
export import pragma.lua;

export {
	namespace pragma::util {
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

	namespace Lua {
		DLLNETWORK std::any GetAnyValue(lua::State *l, pragma::util::VarType varType, int32_t idx);
		DLLNETWORK std::any GetAnyPropertyValue(lua::State *l, int32_t indexProperty, pragma::util::VarType varType);
		DLLNETWORK void SetAnyPropertyValue(lua::State *l, int32_t indexProperty, pragma::util::VarType varType, const std::any &value);
		DLLNETWORK void PushAny(lua::State *l, pragma::util::VarType varType, const std::any &value);
		DLLNETWORK void PushNewAnyProperty(lua::State *l, pragma::util::VarType varType, const std::any &value);

		DLLNETWORK void WriteAny(pragma::util::DataStream &ds, pragma::util::VarType varType, const std::any &value, uint32_t *pos = nullptr);
		DLLNETWORK void WriteAny(NetPacket &ds, pragma::util::VarType varType, const std::any &value, uint32_t *pos = nullptr);
		DLLNETWORK void ReadAny(pragma::Game &game, pragma::util::DataStream &ds, pragma::util::VarType varType, std::any &outValue);
		DLLNETWORK void ReadAny(NetPacket &ds, pragma::util::VarType varType, std::any &outValue);
	};
};
