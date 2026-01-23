// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :networking.util;

void pragma::networking::write_vector(NetPacket &packet, const Vector3 &v)
{
	packet->Write<float>(v.x);
	packet->Write<float>(v.y);
	packet->Write<float>(v.z);
}
void pragma::networking::write_angles(NetPacket &packet, const EulerAngles &ang)
{
	packet->Write<float>(ang.p);
	packet->Write<float>(ang.y);
	packet->Write<float>(ang.r);
}
void pragma::networking::write_quat(NetPacket &packet, const Quat &rot)
{
	packet->Write<float>(rot.w);
	packet->Write<float>(rot.x);
	packet->Write<float>(rot.y);
	packet->Write<float>(rot.z);
}
void pragma::networking::write_entity(NetPacket &packet, const ecs::BaseEntity *ent) { packet->Write<unsigned int>((ent != nullptr) ? ent->GetIndex() : (unsigned int)(-1)); }
void pragma::networking::write_entity(NetPacket &packet, const EntityHandle &hEnt) { write_entity(packet, hEnt.get()); }
Vector3 pragma::networking::read_vector(NetPacket &packet)
{
	Vector3 r(0.f, 0.f, 0.f);
	r.x = packet->Read<float>();
	r.y = packet->Read<float>();
	r.z = packet->Read<float>();
	return r;
}
EulerAngles pragma::networking::read_angles(NetPacket &packet)
{
	EulerAngles ang(0.f, 0.f, 0.f);
	ang.p = packet->Read<float>();
	ang.y = packet->Read<float>();
	ang.r = packet->Read<float>();
	return ang;
}
Quat pragma::networking::read_quat(NetPacket &packet)
{
	auto rot = uquat::identity();
	rot.w = packet->Read<float>();
	rot.x = packet->Read<float>();
	rot.y = packet->Read<float>();
	rot.z = packet->Read<float>();
	return rot;
}

static pragma::ecs::BaseEntity *read_entity(NetPacket &packet, const std::function<void(pragma::ecs::BaseEntity *)> &onCreated = nullptr, CallbackHandle *hCallback = nullptr)
{
	pragma::NetworkState *state;
	if(!packet.IsClient())
		state = pragma::Engine::Get()->GetServerNetworkState();
	else
		state = pragma::Engine::Get()->GetClientState();
	if(state == nullptr || !state->IsGameActive())
		return nullptr;
	auto *game = state->GetGameState();
	auto idx = packet->Read<uint32_t>();
	if(idx == std::numeric_limits<uint32_t>::max())
		return nullptr;
	auto *ent = game->GetEntity(idx);
	if(ent != nullptr) {
		if(onCreated != nullptr)
			onCreated(ent);
		return ent;
	}
	if(onCreated == nullptr)
		return nullptr;
	auto cb = FunctionCallback<void, pragma::ecs::BaseEntity *>::Create(nullptr);
	cb.get<Callback<void, pragma::ecs::BaseEntity *>>()->SetFunction(std::bind(
	  [onCreated, idx](CallbackHandle hCb, pragma::ecs::BaseEntity *ent) {
		  if(ent->GetIndex() != idx)
			  return;
		  onCreated(ent);
		  if(hCb.IsValid())
			  hCb.Remove();
	  },
	  cb, std::placeholders::_1));
	*hCallback = game->AddCallback("OnEntityCreated", cb);
	return nullptr;
}
CallbackHandle pragma::networking::read_entity(NetPacket &packet, const std::function<void(ecs::BaseEntity *)> &onCreated)
{
	CallbackHandle r;
	::read_entity(packet, onCreated, &r);
	return r;
}
pragma::ecs::BaseEntity *pragma::networking::read_entity(NetPacket &packet) { return ::read_entity(packet); }

void pragma::networking::write_player(NetPacket &packet, const ecs::BaseEntity *pl) { write_entity(packet, pl); }
void pragma::networking::write_player(NetPacket &packet, const BasePlayerComponent *plComponent) { write_entity(packet, (plComponent != nullptr) ? dynamic_cast<ecs::BaseEntity *>(plComponent->GetBasePlayer()) : nullptr); }
pragma::BasePlayerComponent *pragma::networking::read_player(NetPacket &packet)
{
	auto *ent = dynamic_cast<BasePlayer *>(::read_entity(packet));
	if(ent == nullptr)
		return nullptr;
	return ent->GetBasePlayerComponent().get();
}
