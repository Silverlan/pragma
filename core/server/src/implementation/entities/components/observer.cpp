// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.observer;

import :entities;
import :server_state;

using namespace pragma;

namespace pragma {
	using ::operator<<;
};

SObserverComponent::SObserverComponent(ecs::BaseEntity &ent) : BaseObserverComponent(ent), SBaseNetComponent() {}

SObserverComponent::~SObserverComponent() {}

void SObserverComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SObserverComponent::DoSetObserverMode(ObserverMode mode)
{
	BaseObserverComponent::DoSetObserverMode(mode);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		p->Write<ObserverMode>(mode);
		ent.SendNetEvent(m_netEvSetObserverMode, p, networking::Protocol::SlowReliable);
	}
}

void SObserverComponent::SetObserverTarget(BaseObservableComponent *ent)
{
	BaseObserverComponent::SetObserverTarget(ent);
	auto &entThis = static_cast<SBaseEntity &>(GetEntity());
	if(entThis.IsShared() == false)
		return;
	NetPacket p {};
	networking::write_entity(p, &ent->GetEntity());
	entThis.SendNetEvent(m_netEvSetObserverTarget, p, networking::Protocol::SlowReliable);
}

void SObserverComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	auto *target = GetObserverTarget();
	networking::write_entity(packet, target ? &target->GetEntity() : nullptr);
	packet->Write<ObserverMode>(GetObserverMode());
}
