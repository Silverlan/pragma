// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


module pragma.client;

import :entities.components.effects.smoke_trail;
import :client_state;
import :engine;
import :util;

using namespace pragma;

CSmokeTrailComponent::~CSmokeTrailComponent() { DestroyParticle(); }
void CSmokeTrailComponent::Initialize()
{
	BaseEnvSmokeTrailComponent::Initialize();
	pragma::ecs::CParticleSystemComponent::Precache("smoke.wpt");
}
void CSmokeTrailComponent::OnEntitySpawn()
{
	BaseEnvSmokeTrailComponent::OnEntitySpawn();
	InitializeParticle();
}
void CSmokeTrailComponent::ReceiveData(NetPacket &packet)
{
	m_speed = packet->Read<float>();
	m_distance = packet->Read<float>();
	m_minSpriteSize = packet->Read<float>();
	m_maxSpriteSize = packet->Read<float>();
	m_material = packet->ReadString();
}
util::EventReply CSmokeTrailComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEnvSmokeTrailComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == BaseToggleComponent::EVENT_ON_TURN_ON)
		InitializeParticle();
	else if(eventId == BaseToggleComponent::EVENT_ON_TURN_OFF)
		DestroyParticle();
	return util::EventReply::Unhandled;
}

void CSmokeTrailComponent::InitializeParticle()
{
	auto &ent = GetEntity();
	auto *pToggleComponent = static_cast<pragma::BaseToggleComponent *>(ent.FindComponent("toggle").get());
	if((pToggleComponent != nullptr && pToggleComponent->IsTurnedOn() == false) || m_hParticle.valid() == true)
		return;
	auto *pt = util::create_smoke_trail_particle(m_distance, m_speed, m_minSpriteSize, m_maxSpriteSize, m_material);
	if(pt == nullptr)
		return;
	pt->Start();
	m_hParticle = pt->GetHandle<pragma::ecs::CParticleSystemComponent>();
}

void CSmokeTrailComponent::DestroyParticle()
{
	if(m_hParticle.valid()) {
		m_hParticle->Die();
		m_hParticle->SetRemoveOnComplete(true);
	}
}
void CSmokeTrailComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////////

void CEnvSmokeTrail::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSmokeTrailComponent>();
}
