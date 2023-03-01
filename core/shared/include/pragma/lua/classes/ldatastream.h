/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LDATASTREAM_H__
#define __LDATASTREAM_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include "luasystem.h"
#include "pragma/lua/luabind_int.h"
#include <mathutil/vertex.hpp>
#include <sharedutils/datastream.h>

lua_registercheck(DataStream, ::DataStream);

namespace Lua {
	namespace DataStream {
		DLLNETWORK void register_class(luabind::class_<::DataStream> &classDef);
		template<class TDataStream>
		void GetSize(lua_State *l, TDataStream &ds)
		{
			Lua::PushInt<size_t>(l, ds->GetInternalSize());
		}
		template<class TDataStream>
		void Seek(lua_State *, TDataStream &ds, uint32_t offset)
		{
			ds->SetOffset(offset);
		}
		template<class TDataStream>
		void Tell(lua_State *l, TDataStream &ds)
		{
			Lua::PushInt(l, ds->GetOffset());
		}
		template<class TDataStream>
		void Resize(lua_State *, TDataStream &ds, uint32_t size)
		{
			ds->Resize(size, true);
		}
		template<class TDataStream>
		void Reserve(lua_State *, TDataStream &ds, uint32_t size)
		{
			ds->Reserve(size);
		}
		template<class TDataStream>
		void Clear(lua_State *, TDataStream &ds)
		{
			ds->Resize(0, true);
		}
		template<class TDataStream>
		void ReadBinaryString(lua_State *l, TDataStream &ds, uint32_t size)
		{
			std::string binaryString {};
			binaryString.resize(size);
			ds->Read(binaryString.data(), size);
			Lua::PushString(l, binaryString);
		}
		template<class TDataStream>
		void ToBinaryString(lua_State *l, TDataStream &ds)
		{
			auto currentOffset = ds->GetOffset();
			ds->SetOffset(0);
			ReadBinaryString(l, ds, ds->GetSize());
			ds->SetOffset(currentOffset);
		}
		template<class TDataStream>
		void WriteString(lua_State *, TDataStream &ds, const std::string &str)
		{
			ds->WriteString(str);
		}
		template<class TDataStream>
		void WriteString(lua_State *, TDataStream &ds, const std::string &str, Bool bNullterminated)
		{
			ds->WriteString(str, bNullterminated);
		}
		template<class TDataStream>
		void WriteBinaryString(lua_State *, TDataStream &ds, const std::string &str)
		{
			ds->Write(reinterpret_cast<const uint8_t *>(str.data()), str.size());
		}
		template<class TDataStream>
		void WriteBinary(lua_State *, TDataStream &ds, TDataStream &dsOther, uint32_t offset, uint32_t size)
		{
			auto *pData = dsOther->GetData() + offset;
			ds->Write(pData, size);
		}
		template<class TDataStream>
		void WriteBinary(lua_State *l, TDataStream &ds, TDataStream &dsOther)
		{
			ds->template Write<uint32_t>(dsOther->GetSize());
			WriteBinary(l, ds, dsOther, 0, dsOther->GetSize());
		}
		template<class TDataStream>
		void ReadBinary(lua_State *l, TDataStream &ds, uint32_t size)
		{
			TDataStream newDs {};
			newDs->Write(ds->GetData() + ds->GetOffset(), size);
			ds->SetOffset(ds->GetOffset() + size);
			Lua::Push<TDataStream>(l, newDs);
		}
		template<class TDataStream>
		void ReadBinary(lua_State *l, TDataStream &ds)
		{
			auto size = ds->template Read<uint32_t>();
			ReadBinary(l, ds, size);
		}
		template<class TDataStream>
		void ReadString(lua_State *l, TDataStream &ds)
		{
			auto str = ds->ReadString();
			Lua::PushString(l, str);
		}
		template<class TDataStream>
		void ReadString(lua_State *l, TDataStream &ds, unsigned int len)
		{
			auto str = ds->ReadString(len);
			Lua::PushString(l, str);
		}
		template<class TDataStream>
		void ReadStringUntil(lua_State *l, TDataStream &ds, const std::string &pattern)
		{
			auto str = ds->ReadUntil(pattern);
			Lua::PushString(l, str);
		}
		template<class TDataStream>
		void ReadLine(lua_State *l, TDataStream &ds)
		{
			auto r = ds->ReadLine();
			Lua::PushString(l, r);
		}
		template<class TDataStream, class TType>
		void WriteData(lua_State *, TDataStream &ds, const TType &t)
		{
			ds->template Write<TType>(t);
		}
		template<class TDataStream, class TLuaType, class TType>
		void Write(lua_State *, TDataStream &ds, TLuaType t)
		{
			ds->template Write<TType>(t);
		}
		template<class TDataStream, class TLuaType, class TType>
		void Read(lua_State *l, TDataStream &ds)
		{
			Lua::Push<TLuaType>(l, static_cast<TLuaType>(ds->template Read<TType>()));
		}
		template<class TDataStream, class TType>
		void Write(lua_State *l, TDataStream &ds, TType t)
		{
			Write<TDataStream, TType, TType>(l, ds, t);
		}
		template<class TDataStream, class TType>
		void Read(lua_State *l, TDataStream &ds)
		{
			Read<TDataStream, TType, TType>(l, ds);
		}
		template<class TClass>
		void register_class(luabind::class_<TClass> &classDef)
		{
			classDef.def(luabind::constructor<>());
			classDef.def(luabind::tostring(luabind::self));
			classDef.def("GetSize", &Lua::DataStream::GetSize<TClass>);
			classDef.def("Seek", &Lua::DataStream::Seek<TClass>);
			classDef.def("Tell", &Lua::DataStream::Tell<TClass>);
			classDef.def("Resize", &Lua::DataStream::Resize<TClass>);
			classDef.def("Reserve", &Lua::DataStream::Reserve<TClass>);
			classDef.def("Clear", &Lua::DataStream::Clear<TClass>);
			classDef.def("ReadBinaryString", static_cast<void (*)(lua_State *, TClass &, uint32_t)>(&Lua::DataStream::ReadBinaryString<TClass>));
			classDef.def("ToBinaryString", static_cast<void (*)(lua_State *, TClass &)>(&Lua::DataStream::ToBinaryString<TClass>));
			classDef.def("WriteString", static_cast<void (*)(lua_State *, TClass &, const std::string &)>(&Lua::DataStream::WriteString<TClass>));
			classDef.def("WriteString", static_cast<void (*)(lua_State *, TClass &, const std::string &, Bool)>(&Lua::DataStream::WriteString<TClass>));
			classDef.def("ReadString", static_cast<void (*)(lua_State *, TClass &)>(&Lua::DataStream::ReadString<TClass>));
			classDef.def("ReadString", static_cast<void (*)(lua_State *, TClass &, unsigned int)>(&Lua::DataStream::ReadString<TClass>));
			classDef.def("ReadStringUntil", &Lua::DataStream::ReadStringUntil<TClass>);
			classDef.def("ReadLine", &Lua::DataStream::ReadLine<TClass>);
			classDef.def("WriteBinary", static_cast<void (*)(lua_State *, TClass &, TClass &, uint32_t, uint32_t)>(&Lua::DataStream::WriteBinary<TClass>));
			classDef.def("WriteBinary", static_cast<void (*)(lua_State *, TClass &, TClass &)>(&Lua::DataStream::WriteBinary<TClass>));
			classDef.def("ReadBinary", static_cast<void (*)(lua_State *, TClass &, uint32_t)>(&Lua::DataStream::ReadBinary<TClass>));
			classDef.def("ReadBinary", static_cast<void (*)(lua_State *, TClass &)>(&Lua::DataStream::ReadBinary<TClass>));
			classDef.def("WriteBool", &Lua::DataStream::Write<TClass, bool>);
			classDef.def("ReadBool", &Lua::DataStream::Read<TClass, bool>);
			classDef.def("WriteInt64", &Lua::DataStream::Write<TClass, int64_t>);
			classDef.def("ReadInt64", &Lua::DataStream::Read<TClass, int64_t>);
			classDef.def("WriteUInt64", &Lua::DataStream::Write<TClass, uint64_t>);
			classDef.def("ReadUInt64", &Lua::DataStream::Read<TClass, uint64_t>);
			classDef.def("WriteInt8", &Lua::DataStream::Write<TClass, int8_t>);
			classDef.def("ReadInt8", &Lua::DataStream::Read<TClass, int8_t>);
			classDef.def("WriteUInt8", &Lua::DataStream::Write<TClass, uint8_t>);
			classDef.def("ReadUInt8", &Lua::DataStream::Read<TClass, uint8_t>);
			classDef.def("WriteInt32", &Lua::DataStream::Write<TClass, int32_t>);
			classDef.def("ReadInt32", &Lua::DataStream::Read<TClass, int32_t>);
			classDef.def("WriteUInt32", &Lua::DataStream::Write<TClass, uint32_t>);
			classDef.def("ReadUInt32", &Lua::DataStream::Read<TClass, uint32_t>);
			classDef.def("WriteInt16", &Lua::DataStream::Write<TClass, int16_t>);
			classDef.def("ReadInt16", &Lua::DataStream::Read<TClass, int16_t>);
			classDef.def("WriteUInt16", &Lua::DataStream::Write<TClass, uint16_t>);
			classDef.def("ReadUInt16", &Lua::DataStream::Read<TClass, uint16_t>);
			classDef.def("WriteChar", &Lua::DataStream::Write<TClass, char>);
			classDef.def("ReadChar", &Lua::DataStream::Read<TClass, char>);
			classDef.def("WriteFloat", &Lua::DataStream::Write<TClass, float>);
			classDef.def("ReadFloat", &Lua::DataStream::Read<TClass, float>);
			classDef.def("WriteDouble", &Lua::DataStream::Write<TClass, double>);
			classDef.def("ReadDouble", &Lua::DataStream::Read<TClass, double>);
			classDef.def("WriteLongDouble", &Lua::DataStream::Write<TClass, long double>);
			classDef.def("ReadLongDouble", &Lua::DataStream::Read<TClass, long double>);
			classDef.def("WriteVector", &Lua::DataStream::WriteData<TClass, Vector3>);
			classDef.def("ReadVector", &Lua::DataStream::Read<TClass, Vector3>);
			classDef.def("WriteVector2", &Lua::DataStream::WriteData<TClass, ::Vector2>);
			classDef.def("ReadVector2", &Lua::DataStream::Read<TClass, ::Vector2>);
			classDef.def("WriteVector4", &Lua::DataStream::WriteData<TClass, ::Vector4>);
			classDef.def("ReadVector4", &Lua::DataStream::Read<TClass, ::Vector4>);
			classDef.def("WriteAngles", &Lua::DataStream::WriteData<TClass, EulerAngles>);
			classDef.def("ReadAngles", &Lua::DataStream::Read<TClass, EulerAngles>);
			classDef.def("WriteMat2", &Lua::DataStream::WriteData<TClass, ::Mat2>);
			classDef.def("WriteMat2x3", &Lua::DataStream::WriteData<TClass, ::Mat2x3>);
			classDef.def("WriteMat2x4", &Lua::DataStream::WriteData<TClass, ::Mat2x4>);
			classDef.def("WriteMat3x2", &Lua::DataStream::WriteData<TClass, ::Mat3x2>);
			classDef.def("WriteMat3", &Lua::DataStream::WriteData<TClass, ::Mat3>);
			classDef.def("WriteMat3x4", &Lua::DataStream::WriteData<TClass, ::Mat3x4>);
			classDef.def("WriteMat4x2", &Lua::DataStream::WriteData<TClass, ::Mat4x2>);
			classDef.def("WriteMat4x3", &Lua::DataStream::WriteData<TClass, ::Mat4x3>);
			classDef.def("WriteMat4", &Lua::DataStream::WriteData<TClass, ::Mat4>);
			classDef.def("WriteQuaternion", &Lua::DataStream::WriteData<TClass, Quat>);
			classDef.def("WriteVector2i", &Lua::DataStream::WriteData<TClass, ::Vector2i>);
			classDef.def("WriteVectori", &Lua::DataStream::WriteData<TClass, Vector3i>);
			classDef.def("WriteVector4i", &Lua::DataStream::WriteData<TClass, ::Vector4i>);
			classDef.def("WriteVertex", &Lua::DataStream::WriteData<TClass, umath::Vertex>);
			classDef.def("ReadMat2", &Lua::DataStream::Read<TClass, ::Mat2>);
			classDef.def("ReadMat2x3", &Lua::DataStream::Read<TClass, ::Mat2x3>);
			classDef.def("ReadMat2x4", &Lua::DataStream::Read<TClass, ::Mat2x4>);
			classDef.def("ReadMat3x2", &Lua::DataStream::Read<TClass, ::Mat3x2>);
			classDef.def("ReadMat3", &Lua::DataStream::Read<TClass, ::Mat3>);
			classDef.def("ReadMat3x4", &Lua::DataStream::Read<TClass, ::Mat3x4>);
			classDef.def("ReadMat4x2", &Lua::DataStream::Read<TClass, ::Mat4x2>);
			classDef.def("ReadMat4x3", &Lua::DataStream::Read<TClass, ::Mat4x3>);
			classDef.def("ReadMat4", &Lua::DataStream::Read<TClass, ::Mat4>);
			classDef.def("ReadQuaternion", &Lua::DataStream::Read<TClass, Quat>);
			classDef.def("ReadVector2i", &Lua::DataStream::Read<TClass, ::Vector2i>);
			classDef.def("ReadVectori", &Lua::DataStream::Read<TClass, Vector3i>);
			classDef.def("ReadVector4i", &Lua::DataStream::Read<TClass, ::Vector4i>);
			classDef.def("ReadVertex", &Lua::DataStream::Read<TClass, umath::Vertex>);
		}
	};
};

#endif
