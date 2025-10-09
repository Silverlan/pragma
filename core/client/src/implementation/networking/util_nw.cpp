// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "sharedutils/functioncallback.h"

#include "stdafx_client.h"

module pragma.client;

import :networking.util;

static CBaseEntity *read_unique_entity(NetPacket &packet, const std::function<void(BaseEntity *)> &onCreated = nullptr, CallbackHandle *hCallback = nullptr)
{
	if(pragma::get_cgame() == nullptr)
		return nullptr;
	auto uuid = packet->Read<util::Uuid>();
	if(uuid == util::Uuid {})
		return nullptr;
	std::vector<BaseEntity *> *ents;
	pragma::get_cgame()->GetEntities(&ents);
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
	*hCallback = pragma::get_cgame()->AddCallback("OnEntitySpawned", cb); // Data is received after entity has been created, and before it has been spawned
	return nullptr;
}
CallbackHandle nwm::read_unique_entity(NetPacket &packet, const std::function<void(BaseEntity *)> &onCreated)
{
	CallbackHandle r;
	::read_unique_entity(packet, onCreated, &r);
	return r;
}
CBaseEntity *nwm::read_unique_entity(NetPacket &packet) { return ::read_unique_entity(packet); }
