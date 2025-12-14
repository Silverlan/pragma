// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

module pragma.shared;

import :scripting.lua.classes.net_packet;

void Lua::NetPacket::register_class(luabind::class_<::NetPacket> &classDef)
{
	Lua::DataStream::register_class<::NetPacket>(classDef);
	classDef.def("WriteEntity", static_cast<void (*)(lua::State *, ::NetPacket &, pragma::ecs::BaseEntity *)>(&WriteEntity));
	classDef.def("WriteEntity", static_cast<void (*)(lua::State *, ::NetPacket &)>(&WriteEntity));
	classDef.def("ReadEntity", &ReadEntity);
	classDef.def("ReadSoundSource", &ReadALSound);
	classDef.def("GetTimeSinceTransmission", &GetTimeSinceTransmission);
}

DLLNETWORK void Lua_NetPacket_GetSize(lua::State *l, NetPacket &packet)
{
	size_t s = packet->GetSize();
	Lua::PushNumber<size_t>(l, s);
}

void Lua_NetPacket_WriteString(lua::State *, NetPacket &packet, const std::string &str) { packet->WriteString(str); }
void Lua_NetPacket_WriteString(lua::State *, NetPacket &packet, const std::string &str, Bool bNullterminated) { packet->WriteString(str, bNullterminated); }

void Lua_NetPacket_ReadString(lua::State *l, NetPacket &packet)
{
	auto str = packet->ReadString();
	Lua::PushString(l, str);
}

void Lua_NetPacket_ReadString(lua::State *l, NetPacket &packet, unsigned int len)
{
	auto str = packet->ReadString(len);
	Lua::PushString(l, str);
}

void Lua_NetPacket_ReadStringUntil(lua::State *l, NetPacket &packet, const std::string &pattern)
{
	auto str = packet->ReadUntil(pattern);
	Lua::PushString(l, str);
}

void Lua_NetPacket_ReadLine(lua::State *l, NetPacket &packet)
{
	auto r = packet->ReadLine();
	Lua::PushString(l, r);
}

void Lua_NetPacket_WriteVector(lua::State *, NetPacket &packet, const Vector3 &v) { pragma::networking::write_vector(packet, v); }

void Lua_NetPacket_ReadVector(lua::State *l, NetPacket &packet)
{
	Vector3 v = pragma::networking::read_vector(packet);
	luabind::object(l, v).push(l);
}

void Lua_NetPacket_WriteVector2(lua::State *, NetPacket &packet, const Vector2 &v)
{
	packet->Write<float>(v.x);
	packet->Write<float>(v.y);
}

void Lua_NetPacket_ReadVector2(lua::State *l, NetPacket &packet)
{
	auto x = packet->Read<float>();
	auto y = packet->Read<float>();
	Lua::Push<Vector2>(l, {x, y});
}

void Lua_NetPacket_WriteVector4(lua::State *, NetPacket &packet, const Vector4 &v)
{
	packet->Write<float>(v.x);
	packet->Write<float>(v.y);
	packet->Write<float>(v.z);
	packet->Write<float>(v.w);
}

void Lua_NetPacket_ReadVector4(lua::State *l, NetPacket &packet)
{
	auto x = packet->Read<float>();
	auto y = packet->Read<float>();
	auto z = packet->Read<float>();
	auto w = packet->Read<float>();
	Lua::Push<Vector4>(l, {x, y, z, w});
}

DLLNETWORK void Lua_NetPacket_WriteAngles(lua::State *, NetPacket &packet, EulerAngles a) { pragma::networking::write_angles(packet, a); }

DLLNETWORK void Lua_NetPacket_ReadAngles(lua::State *l, NetPacket &packet)
{
	EulerAngles a = pragma::networking::read_angles(packet);
	luabind::object(l, a).push(l);
}

void Lua::NetPacket::WriteEntity(lua::State *l, ::NetPacket &packet, pragma::ecs::BaseEntity *hEnt)
{
	if(hEnt == nullptr)
		pragma::networking::write_entity(packet, static_cast<pragma::ecs::BaseEntity *>(nullptr));
	else
		pragma::networking::write_entity(packet, hEnt);
}

void Lua::NetPacket::WriteEntity(lua::State *, ::NetPacket &packet) { pragma::networking::write_entity(packet, nullptr); }

void Lua::NetPacket::ReadEntity(lua::State *l, ::NetPacket &packet)
{
	pragma::ecs::BaseEntity *ent = pragma::networking::read_entity(packet);
	if(ent == nullptr)
		return;
	ent->GetLuaObject().push(l);
}

void Lua::NetPacket::ReadALSound(lua::State *l, ::NetPacket &packet)
{
	unsigned int idx = packet->Read<unsigned int>();
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	std::shared_ptr<pragma::audio::ALSound> als = state->GetSoundByIndex(idx);
	if(als == nullptr)
		return;
	luabind::object(l, als).push(l);
}

void Lua::NetPacket::GetTimeSinceTransmission(lua::State *l, ::NetPacket &packet)
{
	auto tActivated = packet.GetTimeActivated();
	auto tCur = pragma::util::clock::to_int(pragma::util::clock::get_duration_since_start());
	auto t = (tCur - tActivated) / 1'000'000.0;
	Lua::PushNumber(l, t);
}
