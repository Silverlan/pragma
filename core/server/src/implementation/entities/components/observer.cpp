// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include <pragma/entities/components/base_observable_component.hpp>
#include "pragma/entities/s_baseentity.h"
#include <pragma/networking/nwm_util.h>
#include <pragma/networking/enums.hpp>

module pragma.server.entities.components.observer;

import pragma.server.server_state;

using namespace pragma;

namespace pragma {
	using ::operator<<;
};

SObserverComponent::SObserverComponent(BaseEntity &ent) : BaseObserverComponent(ent), SBaseNetComponent() {}

SObserverComponent::~SObserverComponent() {}

void SObserverComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SObserverComponent::DoSetObserverMode(ObserverMode mode)
{
	BaseObserverComponent::DoSetObserverMode(mode);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		p->Write<ObserverMode>(mode);
		ent.SendNetEvent(m_netEvSetObserverMode, p, pragma::networking::Protocol::SlowReliable);
	}
}

void SObserverComponent::SetObserverTarget(BaseObservableComponent *ent)
{
	BaseObserverComponent::SetObserverTarget(ent);
	auto &entThis = static_cast<SBaseEntity &>(GetEntity());
	if(entThis.IsShared() == false)
		return;
	NetPacket p {};
	nwm::write_entity(p, &ent->GetEntity());
	entThis.SendNetEvent(m_netEvSetObserverTarget, p, pragma::networking::Protocol::SlowReliable);
}

void SObserverComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	auto *target = GetObserverTarget();
	nwm::write_entity(packet, target ? &target->GetEntity() : nullptr);
	packet->Write<ObserverMode>(GetObserverMode());
}
