/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include <pragma/engine.h>
#include <pragma/game/game.h>
#include "pragma/networkstate/networkstate.h"
#include <sharedutils/netpacket.hpp>
#include "pragma/networking/nwm_util.h"
#include <pragma/math/angle/wvangle.h>
#include "pragma/entities/baseentity.h"
#include "pragma/entities/baseentity_handle.h"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/entities/baseplayer.hpp"

extern DLLNETWORK Engine *engine;

void nwm::write_vector(NetPacket &packet, const Vector3 &v)
{
	packet->Write<float>(v.x);
	packet->Write<float>(v.y);
	packet->Write<float>(v.z);
}
void nwm::write_angles(NetPacket &packet, const EulerAngles &ang)
{
	packet->Write<float>(ang.p);
	packet->Write<float>(ang.y);
	packet->Write<float>(ang.r);
}
void nwm::write_quat(NetPacket &packet, const Quat &rot)
{
	packet->Write<float>(rot.w);
	packet->Write<float>(rot.x);
	packet->Write<float>(rot.y);
	packet->Write<float>(rot.z);
}
void nwm::write_entity(NetPacket &packet, const BaseEntity *ent) { packet->Write<unsigned int>((ent != nullptr) ? ent->GetIndex() : (unsigned int)(-1)); }
void nwm::write_entity(NetPacket &packet, const EntityHandle &hEnt) { write_entity(packet, hEnt.get()); }
Vector3 nwm::read_vector(NetPacket &packet)
{
	Vector3 r(0.f, 0.f, 0.f);
	r.x = packet->Read<float>();
	r.y = packet->Read<float>();
	r.z = packet->Read<float>();
	return r;
}
EulerAngles nwm::read_angles(NetPacket &packet)
{
	EulerAngles ang(0.f, 0.f, 0.f);
	ang.p = packet->Read<float>();
	ang.y = packet->Read<float>();
	ang.r = packet->Read<float>();
	return ang;
}
Quat nwm::read_quat(NetPacket &packet)
{
	auto rot = uquat::identity();
	rot.w = packet->Read<float>();
	rot.x = packet->Read<float>();
	rot.y = packet->Read<float>();
	rot.z = packet->Read<float>();
	return rot;
}

static BaseEntity *read_entity(NetPacket &packet, const std::function<void(BaseEntity *)> &onCreated = nullptr, CallbackHandle *hCallback = nullptr)
{
	NetworkState *state;
	if(!packet.IsClient())
		state = engine->GetServerNetworkState();
	else
		state = engine->GetClientState();
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
	auto cb = FunctionCallback<void, BaseEntity *>::Create(nullptr);
	cb.get<Callback<void, BaseEntity *>>()->SetFunction(std::bind(
	  [onCreated, idx](CallbackHandle hCb, BaseEntity *ent) {
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
CallbackHandle nwm::read_entity(NetPacket &packet, const std::function<void(BaseEntity *)> &onCreated)
{
	CallbackHandle r;
	::read_entity(packet, onCreated, &r);
	return r;
}
BaseEntity *nwm::read_entity(NetPacket &packet) { return ::read_entity(packet); }

void nwm::write_player(NetPacket &packet, const BaseEntity *pl) { write_entity(packet, pl); }
void nwm::write_player(NetPacket &packet, const pragma::BasePlayerComponent *plComponent) { write_entity(packet, (plComponent != nullptr) ? dynamic_cast<BaseEntity *>(plComponent->GetBasePlayer()) : nullptr); }
pragma::BasePlayerComponent *nwm::read_player(NetPacket &packet)
{
	auto *ent = dynamic_cast<BasePlayer *>(::read_entity(packet));
	if(ent == nullptr)
		return nullptr;
	return ent->GetBasePlayerComponent().get();
}
