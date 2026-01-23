// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.point_at_target;

import :entities.base;
import :networking.util;

using namespace pragma;

void SPointAtTargetComponent::Initialize() { BasePointAtTargetComponent::Initialize(); }
void SPointAtTargetComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SPointAtTargetComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { networking::write_unique_entity(packet, GetPointAtTarget()); }

void SPointAtTargetComponent::SetPointAtTarget(ecs::BaseEntity *ent)
{
	BasePointAtTargetComponent::SetPointAtTarget(ent);
	NetPacket p {};
	networking::write_entity(p, ent);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetPointAtTarget, p, networking::Protocol::SlowReliable);
}
