/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan */

#include "stdafx_server.h"
#include "pragma/serverstate/serverutil.h"
#include "pragma/entities/components/s_observer_component.hpp"
#include <pragma/entities/components/base_observable_component.hpp>
#include <pragma/networking/nwm_util.h>

using namespace pragma;

namespace pragma {
	using ::operator<<;
};

extern ServerState *server;

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
