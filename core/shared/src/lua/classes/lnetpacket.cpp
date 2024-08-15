/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/lnetpacket.h"
#include <pragma/engine.h>
#include "pragma/networking/nwm_util.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "luasystem.h"
#include <sharedutils/util_clock.hpp>

extern DLLNETWORK Engine *engine;

void Lua::NetPacket::register_class(luabind::class_<::NetPacket> &classDef)
{
	Lua::DataStream::register_class<::NetPacket>(classDef);
	classDef.def("WriteEntity", static_cast<void (*)(lua_State *, ::NetPacket &, BaseEntity *)>(&Lua::NetPacket::WriteEntity));
	classDef.def("WriteEntity", static_cast<void (*)(lua_State *, ::NetPacket &)>(&Lua::NetPacket::WriteEntity));
	classDef.def("ReadEntity", &Lua::NetPacket::ReadEntity);
	classDef.def("ReadSoundSource", &Lua::NetPacket::ReadALSound);
	classDef.def("GetTimeSinceTransmission", &Lua::NetPacket::GetTimeSinceTransmission);
}

DLLNETWORK void Lua_NetPacket_GetSize(lua_State *l, NetPacket &packet)
{
	size_t s = packet->GetSize();
	Lua::PushNumber<size_t>(l, s);
}

void Lua_NetPacket_WriteString(lua_State *, NetPacket &packet, const std::string &str) { packet->WriteString(str); }
void Lua_NetPacket_WriteString(lua_State *, NetPacket &packet, const std::string &str, Bool bNullterminated) { packet->WriteString(str, bNullterminated); }

void Lua_NetPacket_ReadString(lua_State *l, NetPacket &packet)
{
	auto str = packet->ReadString();
	Lua::PushString(l, str);
}

void Lua_NetPacket_ReadString(lua_State *l, NetPacket &packet, unsigned int len)
{
	auto str = packet->ReadString(len);
	Lua::PushString(l, str);
}

void Lua_NetPacket_ReadStringUntil(lua_State *l, NetPacket &packet, const std::string &pattern)
{
	auto str = packet->ReadUntil(pattern);
	Lua::PushString(l, str);
}

void Lua_NetPacket_ReadLine(lua_State *l, NetPacket &packet)
{
	auto r = packet->ReadLine();
	Lua::PushString(l, r);
}

void Lua_NetPacket_WriteVector(lua_State *, NetPacket &packet, const Vector3 &v) { nwm::write_vector(packet, v); }

void Lua_NetPacket_ReadVector(lua_State *l, NetPacket &packet)
{
	Vector3 v = nwm::read_vector(packet);
	luabind::object(l, v).push(l);
}

void Lua_NetPacket_WriteVector2(lua_State *, NetPacket &packet, const Vector2 &v)
{
	packet->Write<float>(v.x);
	packet->Write<float>(v.y);
}

void Lua_NetPacket_ReadVector2(lua_State *l, NetPacket &packet)
{
	auto x = packet->Read<float>();
	auto y = packet->Read<float>();
	Lua::Push<Vector2>(l, {x, y});
}

void Lua_NetPacket_WriteVector4(lua_State *, NetPacket &packet, const Vector4 &v)
{
	packet->Write<float>(v.x);
	packet->Write<float>(v.y);
	packet->Write<float>(v.z);
	packet->Write<float>(v.w);
}

void Lua_NetPacket_ReadVector4(lua_State *l, NetPacket &packet)
{
	auto x = packet->Read<float>();
	auto y = packet->Read<float>();
	auto z = packet->Read<float>();
	auto w = packet->Read<float>();
	Lua::Push<Vector4>(l, {x, y, z, w});
}

DLLNETWORK void Lua_NetPacket_WriteAngles(lua_State *, NetPacket &packet, EulerAngles a) { nwm::write_angles(packet, a); }

DLLNETWORK void Lua_NetPacket_ReadAngles(lua_State *l, NetPacket &packet)
{
	EulerAngles a = nwm::read_angles(packet);
	luabind::object(l, a).push(l);
}

void Lua::NetPacket::WriteEntity(lua_State *l, ::NetPacket &packet, BaseEntity *hEnt)
{
	if(hEnt == nullptr)
		nwm::write_entity(packet, static_cast<BaseEntity *>(nullptr));
	else
		nwm::write_entity(packet, hEnt);
}

void Lua::NetPacket::WriteEntity(lua_State *, ::NetPacket &packet) { nwm::write_entity(packet, nullptr); }

void Lua::NetPacket::ReadEntity(lua_State *l, ::NetPacket &packet)
{
	BaseEntity *ent = nwm::read_entity(packet);
	if(ent == nullptr)
		return;
	lua_pushentity(l, ent);
}

void Lua::NetPacket::ReadALSound(lua_State *l, ::NetPacket &packet)
{
	unsigned int idx = packet->Read<unsigned int>();
	NetworkState *state = engine->GetNetworkState(l);
	std::shared_ptr<ALSound> als = state->GetSoundByIndex(idx);
	if(als == NULL)
		return;
	luabind::object(l, als).push(l);
}

void Lua::NetPacket::GetTimeSinceTransmission(lua_State *l, ::NetPacket &packet)
{
	auto tActivated = packet.GetTimeActivated();
	auto tCur = util::clock::to_int(util::clock::get_duration_since_start());
	auto t = (tCur - tActivated) / 1'000'000.0;
	Lua::PushNumber(l, t);
}
