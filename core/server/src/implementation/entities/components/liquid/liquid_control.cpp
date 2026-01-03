// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.liquid_control;

import :entities;

using namespace pragma;

void SLiquidControlComponent::Initialize() { BaseLiquidControlComponent::Initialize(); }

void SLiquidControlComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SLiquidControlComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->WriteString(m_kvSurfaceMaterial); }

void SLiquidControlComponent::CreateSplash(const Vector3 &origin, float radius, float force)
{
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent == nullptr || pPhysComponent->GetPhysicsObject() == nullptr)
		return;
	BaseLiquidControlComponent::CreateSplash(origin, radius, force);
	if(ent.IsShared() == false)
		return;
	NetPacket packet {};
	packet->Write<Vector3>(origin);
	packet->Write<float>(radius);
	packet->Write<float>(force);
	ent.SendNetEvent(m_netEvCreateSplash, packet, networking::Protocol::SlowReliable);
}
