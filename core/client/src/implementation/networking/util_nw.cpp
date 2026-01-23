// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :networking.util;

static pragma::ecs::CBaseEntity *read_unique_entity(NetPacket &packet, const std::function<void(pragma::ecs::BaseEntity *)> &onCreated = nullptr, CallbackHandle *hCallback = nullptr)
{
	if(pragma::get_cgame() == nullptr)
		return nullptr;
	auto uuid = packet->Read<pragma::util::Uuid>();
	if(uuid == pragma::util::Uuid {})
		return nullptr;
	std::vector<pragma::ecs::BaseEntity *> *ents;
	pragma::get_cgame()->GetEntities(&ents);
	for(auto *ent : *ents) {
		if(ent != nullptr && ent->IsSpawned() == true && ent->GetUuid() == uuid) {
			if(onCreated != nullptr)
				onCreated(ent);
			return static_cast<pragma::ecs::CBaseEntity *>(ent);
		}
	}
	if(onCreated == nullptr)
		return nullptr;
	auto cb = FunctionCallback<void, pragma::ecs::BaseEntity *>::Create(nullptr);
	cb.get<Callback<void, pragma::ecs::BaseEntity *>>()->SetFunction(std::bind(
	  [onCreated, uuid](CallbackHandle hCb, pragma::ecs::BaseEntity *ent) {
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
CallbackHandle pragma::networking::read_unique_entity(NetPacket &packet, const std::function<void(ecs::BaseEntity *)> &onCreated)
{
	CallbackHandle r;
	::read_unique_entity(packet, onCreated, &r);
	return r;
}
pragma::ecs::CBaseEntity *pragma::networking::read_unique_entity(NetPacket &packet) { return ::read_unique_entity(packet); }
