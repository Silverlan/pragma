// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include <pragma/networking/enums.hpp>
#include "sharedutils/netpacket.hpp"
#include "pragma/game/damagetype.h"
#include "pragma/networking/recipient_filter.hpp"
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/components/damageable_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/libraries/ltimer.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/networking/nwm_util.h>

module pragma.server.entities.components.flammable;

import pragma.server.entities;
import pragma.server.game;

using namespace pragma;

SFlammableComponent::IgniteInfo::IgniteInfo() : damageTimer() {}

SFlammableComponent::IgniteInfo::~IgniteInfo() { Clear(); }

void SFlammableComponent::IgniteInfo::Clear()
{
	if(damageTimer != nullptr && damageTimer->IsValid())
		damageTimer->GetTimer()->Remove(SGame::Get());
	hAttacker = EntityHandle {};
	hInflictor = EntityHandle {};
}

/////////////////////////

void SFlammableComponent::ApplyIgnitionDamage()
{
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	auto pDamageableComponent = ent.GetComponent<pragma::DamageableComponent>();
	if(pTrComponent == nullptr || pDamageableComponent.expired())
		return;
	auto pos = ent.GetCenter();
	DamageInfo info {};
	info.SetAttacker(m_igniteInfo.hAttacker.get());
	info.SetInflictor(m_igniteInfo.hInflictor.get());
	info.SetDamageType(DAMAGETYPE::IGNITE);
	info.SetHitPosition(pos);
	info.SetSource(pos);
	info.SetDamage(5);
	pDamageableComponent->TakeDamage(info);
}
void SFlammableComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
util::EventReply SFlammableComponent::Ignite(float duration, BaseEntity *attacker, BaseEntity *inflictor)
{
	if(!IsIgnitable())
		return util::EventReply::Handled;
	NetPacket p {};
	p->Write<float>(duration);
	nwm::write_entity(p, attacker);
	nwm::write_entity(p, inflictor);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvIgnite, p, pragma::networking::Protocol::SlowReliable);

	auto reps = static_cast<uint32_t>(umath::floor(duration / 0.5f));
	Timer *t = nullptr;
	if(IsOnFire() && m_igniteInfo.damageTimer != nullptr && m_igniteInfo.damageTimer->IsValid())
		t = m_igniteInfo.damageTimer->GetTimer();
	if(t == nullptr) {
		t = SGame::Get()->CreateTimer(0.5f, reps, FunctionCallback<void>::Create([this]() { ApplyIgnitionDamage(); }));
		t->Start(SGame::Get());
	}
	else if(reps > t->GetRepetitionsLeft())
		t->SetRepetitions(reps);

	m_igniteInfo.damageTimer = t->CreateHandle();
	m_igniteInfo.hAttacker = (attacker != nullptr) ? attacker->GetHandle() : EntityHandle {};
	m_igniteInfo.hInflictor = (inflictor != nullptr) ? inflictor->GetHandle() : EntityHandle {};

	return BaseFlammableComponent::Ignite(duration, attacker, inflictor);
}
void SFlammableComponent::Extinguish()
{
	if(!IsOnFire())
		return;
	BaseFlammableComponent::Extinguish();
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvExtinguish, pragma::networking::Protocol::SlowReliable);
	m_igniteInfo.Clear();
}
void SFlammableComponent::SetIgnitable(bool b)
{
	if(b == IsIgnitable())
		return;
	BaseFlammableComponent::SetIgnitable(b);
	NetPacket p {};
	p->Write<bool>(b);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetIgnitable, p, pragma::networking::Protocol::SlowReliable);
}
void SFlammableComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<bool>(*m_bIgnitable);
	packet->Write<bool>(*m_bIsOnFire);
	if(*m_bIsOnFire == true) {
		auto dur = (m_igniteInfo.damageTimer->IsValid()) ? m_igniteInfo.damageTimer->GetTimer()->GetTimeLeft() : 0.f;
		packet->Write<float>(dur);
		nwm::write_entity(packet, m_igniteInfo.hAttacker.get());
		nwm::write_entity(packet, m_igniteInfo.hInflictor.get());
	}
}
