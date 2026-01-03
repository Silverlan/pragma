// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.flammable;

import :entities;
import :game;

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
	auto pDamageableComponent = ent.GetComponent<DamageableComponent>();
	if(pTrComponent == nullptr || pDamageableComponent.expired())
		return;
	auto pos = ent.GetCenter();
	game::DamageInfo info {};
	info.SetAttacker(m_igniteInfo.hAttacker.get());
	info.SetInflictor(m_igniteInfo.hInflictor.get());
	info.SetDamageType(DamageType::Ignite);
	info.SetHitPosition(pos);
	info.SetSource(pos);
	info.SetDamage(5);
	pDamageableComponent->TakeDamage(info);
}
void SFlammableComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
util::EventReply SFlammableComponent::Ignite(float duration, ecs::BaseEntity *attacker, ecs::BaseEntity *inflictor)
{
	if(!IsIgnitable())
		return util::EventReply::Handled;
	NetPacket p {};
	p->Write<float>(duration);
	networking::write_entity(p, attacker);
	networking::write_entity(p, inflictor);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvIgnite, p, networking::Protocol::SlowReliable);

	auto reps = static_cast<uint32_t>(math::floor(duration / 0.5f));
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
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvExtinguish, networking::Protocol::SlowReliable);
	m_igniteInfo.Clear();
}
void SFlammableComponent::SetIgnitable(bool b)
{
	if(b == IsIgnitable())
		return;
	BaseFlammableComponent::SetIgnitable(b);
	NetPacket p {};
	p->Write<bool>(b);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetIgnitable, p, networking::Protocol::SlowReliable);
}
void SFlammableComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<bool>(*m_bIgnitable);
	packet->Write<bool>(*m_bIsOnFire);
	if(*m_bIsOnFire == true) {
		auto dur = (m_igniteInfo.damageTimer->IsValid()) ? m_igniteInfo.damageTimer->GetTimer()->GetTimeLeft() : 0.f;
		packet->Write<float>(dur);
		networking::write_entity(packet, m_igniteInfo.hAttacker.get());
		networking::write_entity(packet, m_igniteInfo.hInflictor.get());
	}
}
