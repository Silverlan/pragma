/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/networking/c_nwm_util.h"

extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

static CBaseEntity *read_unique_entity(NetPacket &packet, const std::function<void(BaseEntity *)> &onCreated = nullptr, CallbackHandle *hCallback = nullptr)
{
	if(c_game == nullptr)
		return nullptr;
	auto uuid = packet->Read<util::Uuid>();
	if(uuid == util::Uuid {})
		return nullptr;
	std::vector<BaseEntity *> *ents;
	c_game->GetEntities(&ents);
	for(auto *ent : *ents) {
		if(ent != nullptr && ent->IsSpawned() == true && ent->GetUuid() == uuid) {
			if(onCreated != nullptr)
				onCreated(ent);
			return static_cast<CBaseEntity *>(ent);
		}
	}
	if(onCreated == nullptr)
		return nullptr;
	auto cb = FunctionCallback<void, BaseEntity *>::Create(nullptr);
	cb.get<Callback<void, BaseEntity *>>()->SetFunction(std::bind(
	  [onCreated, uuid](CallbackHandle hCb, BaseEntity *ent) {
		  if(ent->GetUuid() != uuid)
			  return;
		  onCreated(ent);
		  if(hCb.IsValid())
			  hCb.Remove();
	  },
	  cb, std::placeholders::_1));
	*hCallback = c_game->AddCallback("OnEntitySpawned", cb); // Data is received after entity has been created, and before it has been spawned
	return nullptr;
}
CallbackHandle nwm::read_unique_entity(NetPacket &packet, const std::function<void(BaseEntity *)> &onCreated)
{
	CallbackHandle r;
	::read_unique_entity(packet, onCreated, &r);
	return r;
}
CBaseEntity *nwm::read_unique_entity(NetPacket &packet) { return ::read_unique_entity(packet); }
