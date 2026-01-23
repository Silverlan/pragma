// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.data_stream;

export import pragma.lua;

export namespace Lua {
	namespace DataStream {
		DLLNETWORK void register_class(luabind::class_<pragma::util::DataStream> &classDef);
		template<class TDataStream>
		void GetSize(lua::State *l, TDataStream &ds)
		{
			Lua::PushInt<size_t>(l, ds->GetInternalSize());
		}
		template<class TDataStream>
		void Seek(lua::State *, TDataStream &ds, uint32_t offset)
		{
			ds->SetOffset(offset);
		}
		template<class TDataStream>
		void Tell(lua::State *l, TDataStream &ds)
		{
			Lua::PushInt(l, ds->GetOffset());
		}
		template<class TDataStream>
		void Resize(lua::State *, TDataStream &ds, uint32_t size)
		{
			ds->Resize(size, true);
		}
		template<class TDataStream>
		void Reserve(lua::State *, TDataStream &ds, uint32_t size)
		{
			ds->Reserve(size);
		}
		template<class TDataStream>
		void Clear(lua::State *, TDataStream &ds)
		{
			ds->Resize(0, true);
		}
		template<class TDataStream>
		void ReadBinaryString(lua::State *l, TDataStream &ds, uint32_t size)
		{
			std::string binaryString {};
			binaryString.resize(size);
			ds->Read(binaryString.data(), size);
			PushString(l, binaryString);
		}
		template<class TDataStream>
		void ToBinaryString(lua::State *l, TDataStream &ds)
		{
			auto currentOffset = ds->GetOffset();
			ds->SetOffset(0);
			ReadBinaryString(l, ds, ds->GetSize());
			ds->SetOffset(currentOffset);
		}
		template<class TDataStream>
		void WriteString(lua::State *, TDataStream &ds, const std::string &str)
		{
			ds->WriteString(str);
		}
		template<class TDataStream>
		void WriteString(lua::State *, TDataStream &ds, const std::string &str, Bool bNullterminated)
		{
			ds->WriteString(str, bNullterminated);
		}
		template<class TDataStream>
		void WriteBinaryString(lua::State *, TDataStream &ds, const std::string &str)
		{
			ds->Write(reinterpret_cast<const uint8_t *>(str.data()), str.size());
		}
		template<class TDataStream>
		void WriteBinary(lua::State *, TDataStream &ds, TDataStream &dsOther, uint32_t offset, uint32_t size)
		{
			auto *pData = dsOther->GetData() + offset;
			ds->Write(pData, size);
		}
		template<class TDataStream>
		void WriteBinary(lua::State *l, TDataStream &ds, TDataStream &dsOther)
		{
			ds->template Write<uint32_t>(dsOther->GetSize());
			WriteBinary(l, ds, dsOther, 0, dsOther->GetSize());
		}
		template<class TDataStream>
		void ReadBinary(lua::State *l, TDataStream &ds, uint32_t size)
		{
			TDataStream newDs {};
			newDs->Write(ds->GetData() + ds->GetOffset(), size);
			ds->SetOffset(ds->GetOffset() + size);
			Lua::Push<TDataStream>(l, newDs);
		}
		template<class TDataStream>
		void ReadBinary(lua::State *l, TDataStream &ds)
		{
			auto size = ds->template Read<uint32_t>();
			ReadBinary(l, ds, size);
		}
		template<class TDataStream>
		void ReadString(lua::State *l, TDataStream &ds)
		{
			auto str = ds->ReadString();
			Lua::PushString(l, str);
		}
		template<class TDataStream>
		void ReadString(lua::State *l, TDataStream &ds, unsigned int len)
		{
			auto str = ds->ReadString(len);
			Lua::PushString(l, str);
		}
		template<class TDataStream>
		void ReadStringUntil(lua::State *l, TDataStream &ds, const std::string &pattern)
		{
			auto str = ds->ReadUntil(pattern);
			Lua::PushString(l, str);
		}
		template<class TDataStream>
		void ReadLine(lua::State *l, TDataStream &ds)
		{
			auto r = ds->ReadLine();
			Lua::PushString(l, r);
		}
		template<class TDataStream, class TType>
		void WriteData(lua::State *, TDataStream &ds, const TType &t)
		{
			ds->template Write<TType>(t);
		}
		template<class TDataStream, class TLuaType, class TType>
		void Write(lua::State *, TDataStream &ds, TLuaType t)
		{
			ds->template Write<TType>(t);
		}
		template<class TDataStream, class TLuaType, class TType>
		void Read(lua::State *l, TDataStream &ds)
		{
			Lua::Push<TLuaType>(l, static_cast<TLuaType>(ds->template Read<TType>()));
		}
		template<class TDataStream, class TType>
		void Write(lua::State *l, TDataStream &ds, TType t)
		{
			Write<TDataStream, TType, TType>(l, ds, t);
		}
		template<class TDataStream, class TType>
		void Read(lua::State *l, TDataStream &ds)
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
			classDef.def("ReadBinaryString", static_cast<void (*)(lua::State *, TClass &, uint32_t)>(&Lua::DataStream::ReadBinaryString<TClass>));
			classDef.def("ToBinaryString", static_cast<void (*)(lua::State *, TClass &)>(&Lua::DataStream::ToBinaryString<TClass>));
			classDef.def("WriteString", static_cast<void (*)(lua::State *, TClass &, const std::string &)>(&Lua::DataStream::WriteString<TClass>));
			classDef.def("WriteString", static_cast<void (*)(lua::State *, TClass &, const std::string &, Bool)>(&Lua::DataStream::WriteString<TClass>));
			classDef.def("ReadString", static_cast<void (*)(lua::State *, TClass &)>(&Lua::DataStream::ReadString<TClass>));
			classDef.def("ReadString", static_cast<void (*)(lua::State *, TClass &, unsigned int)>(&Lua::DataStream::ReadString<TClass>));
			classDef.def("ReadStringUntil", &Lua::DataStream::ReadStringUntil<TClass>);
			classDef.def("ReadLine", &Lua::DataStream::ReadLine<TClass>);
			classDef.def("WriteBinary", static_cast<void (*)(lua::State *, TClass &, TClass &, uint32_t, uint32_t)>(&Lua::DataStream::WriteBinary<TClass>));
			classDef.def("WriteBinary", static_cast<void (*)(lua::State *, TClass &, TClass &)>(&Lua::DataStream::WriteBinary<TClass>));
			classDef.def("ReadBinary", static_cast<void (*)(lua::State *, TClass &, uint32_t)>(&Lua::DataStream::ReadBinary<TClass>));
			classDef.def("ReadBinary", static_cast<void (*)(lua::State *, TClass &)>(&Lua::DataStream::ReadBinary<TClass>));
			classDef.def("WriteBool", &DataStream::Write<TClass, bool>);
			classDef.def("ReadBool", &DataStream::Read<TClass, bool>);
			classDef.def("WriteInt64", &DataStream::Write<TClass, int64_t>);
			classDef.def("ReadInt64", &DataStream::Read<TClass, int64_t>);
			classDef.def("WriteUInt64", &DataStream::Write<TClass, uint64_t>);
			classDef.def("ReadUInt64", &DataStream::Read<TClass, uint64_t>);
			classDef.def("WriteInt8", &DataStream::Write<TClass, int8_t>);
			classDef.def("ReadInt8", &DataStream::Read<TClass, int8_t>);
			classDef.def("WriteUInt8", &DataStream::Write<TClass, uint8_t>);
			classDef.def("ReadUInt8", &DataStream::Read<TClass, uint8_t>);
			classDef.def("WriteInt32", &DataStream::Write<TClass, int32_t>);
			classDef.def("ReadInt32", &DataStream::Read<TClass, int32_t>);
			classDef.def("WriteUInt32", &DataStream::Write<TClass, uint32_t>);
			classDef.def("ReadUInt32", &DataStream::Read<TClass, uint32_t>);
			classDef.def("WriteInt16", &DataStream::Write<TClass, int16_t>);
			classDef.def("ReadInt16", &DataStream::Read<TClass, int16_t>);
			classDef.def("WriteUInt16", &DataStream::Write<TClass, uint16_t>);
			classDef.def("ReadUInt16", &DataStream::Read<TClass, uint16_t>);
			classDef.def("WriteChar", &DataStream::Write<TClass, char>);
			classDef.def("ReadChar", &DataStream::Read<TClass, char>);
			classDef.def("WriteFloat", &DataStream::Write<TClass, float>);
			classDef.def("ReadFloat", &DataStream::Read<TClass, float>);
			classDef.def("WriteDouble", &DataStream::Write<TClass, double>);
			classDef.def("ReadDouble", &DataStream::Read<TClass, double>);
			classDef.def("WriteLongDouble", &DataStream::Write<TClass, long double>);
			classDef.def("ReadLongDouble", &DataStream::Read<TClass, long double>);
			classDef.def("WriteVector", &Lua::DataStream::WriteData<TClass, Vector3>);
			classDef.def("ReadVector", &DataStream::Read<TClass, Vector3>);
			classDef.def("WriteVector2", &Lua::DataStream::WriteData<TClass, Vector2>);
			classDef.def("ReadVector2", &DataStream::Read<TClass, Vector2>);
			classDef.def("WriteVector4", &Lua::DataStream::WriteData<TClass, Vector4>);
			classDef.def("ReadVector4", &DataStream::Read<TClass, Vector4>);
			classDef.def("WriteAngles", &Lua::DataStream::WriteData<TClass, EulerAngles>);
			classDef.def("ReadAngles", &DataStream::Read<TClass, EulerAngles>);
			classDef.def("WriteMat2", &Lua::DataStream::WriteData<TClass, Mat2>);
			classDef.def("WriteMat2x3", &Lua::DataStream::WriteData<TClass, Mat2x3>);
			classDef.def("WriteMat2x4", &Lua::DataStream::WriteData<TClass, Mat2x4>);
			classDef.def("WriteMat3x2", &Lua::DataStream::WriteData<TClass, Mat3x2>);
			classDef.def("WriteMat3", &Lua::DataStream::WriteData<TClass, Mat3>);
			classDef.def("WriteMat3x4", &Lua::DataStream::WriteData<TClass, Mat3x4>);
			classDef.def("WriteMat4x2", &Lua::DataStream::WriteData<TClass, Mat4x2>);
			classDef.def("WriteMat4x3", &Lua::DataStream::WriteData<TClass, Mat4x3>);
			classDef.def("WriteMat4", &Lua::DataStream::WriteData<TClass, Mat4>);
			classDef.def("WriteQuaternion", &Lua::DataStream::WriteData<TClass, Quat>);
			classDef.def("WriteVector2i", &Lua::DataStream::WriteData<TClass, Vector2i>);
			classDef.def("WriteVectori", &Lua::DataStream::WriteData<TClass, Vector3i>);
			classDef.def("WriteVector4i", &Lua::DataStream::WriteData<TClass, Vector4i>);
			classDef.def("WriteVertex", &Lua::DataStream::WriteData<TClass, pragma::math::Vertex>);
			classDef.def("ReadMat2", &DataStream::Read<TClass, Mat2>);
			classDef.def("ReadMat2x3", &DataStream::Read<TClass, Mat2x3>);
			classDef.def("ReadMat2x4", &DataStream::Read<TClass, Mat2x4>);
			classDef.def("ReadMat3x2", &DataStream::Read<TClass, Mat3x2>);
			classDef.def("ReadMat3", &DataStream::Read<TClass, Mat3>);
			classDef.def("ReadMat3x4", &DataStream::Read<TClass, Mat3x4>);
			classDef.def("ReadMat4x2", &DataStream::Read<TClass, Mat4x2>);
			classDef.def("ReadMat4x3", &DataStream::Read<TClass, Mat4x3>);
			classDef.def("ReadMat4", &DataStream::Read<TClass, Mat4>);
			classDef.def("ReadQuaternion", &DataStream::Read<TClass, Quat>);
			classDef.def("ReadVector2i", &DataStream::Read<TClass, Vector2i>);
			classDef.def("ReadVectori", &DataStream::Read<TClass, Vector3i>);
			classDef.def("ReadVector4i", &DataStream::Read<TClass, Vector4i>);
			classDef.def("ReadVertex", &DataStream::Read<TClass, pragma::math::Vertex>);
		}
	};
};
