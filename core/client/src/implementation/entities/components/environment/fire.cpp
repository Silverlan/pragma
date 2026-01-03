// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.effects.fire;
using namespace pragma;

CFireComponent::~CFireComponent() { DestroyParticle(); }
void CFireComponent::Initialize()
{
	BaseEnvFireComponent::Initialize();
	ecs::CParticleSystemComponent::Precache("fire.wpt");
}
void CFireComponent::ReceiveData(NetPacket &packet) { m_fireType = packet->ReadString(); }
util::EventReply CFireComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEnvFireComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == baseToggleComponent::EVENT_ON_TURN_ON)
		InitializeParticle();
	else if(eventId == baseToggleComponent::EVENT_ON_TURN_OFF)
		DestroyParticle();
	return util::EventReply::Unhandled;
}
void CFireComponent::InitializeParticle()
{
	auto &ent = GetEntity();
	auto *pToggleComponent = static_cast<BaseToggleComponent *>(ent.FindComponent("toggle").get());
	if((pToggleComponent != nullptr && pToggleComponent->IsTurnedOn() == false) || m_hParticle.valid() == true)
		return;
	auto *pt = ecs::CParticleSystemComponent::Create(m_fireType);
	if(pt == nullptr)
		return;
	pt->SetContinuous(true);
	pt->Start();
	m_hParticle = pt->GetHandle<ecs::CParticleSystemComponent>();
}

void CFireComponent::DestroyParticle()
{
	if(m_hParticle.valid()) {
		m_hParticle->Die();
		m_hParticle->SetRemoveOnComplete(true);
	}
}
void CFireComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////////

void CEnvFire::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CFireComponent>();
}
