// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.listener;
import :engine;
import :entities.components.transform;

using namespace pragma;

void CListenerComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	auto &ent = GetEntity();
	ent.AddComponent<CTransformComponent>();
	auto *soundSys = get_cengine()->GetSoundSystem();
	if(soundSys == nullptr) {
		ent.RemoveSafely();
		return;
	}
	m_listener = &soundSys->GetListener();

	SetTickPolicy(TickPolicy::Always);
}

void CListenerComponent::OnTick(double dt)
{
	if(m_listener == nullptr)
		return;
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	auto pVelComponent = ent.GetComponent<VelocityComponent>();
	if(pTrComponent != nullptr)
		m_listener->SetPosition(pTrComponent->GetPosition());
	if(pVelComponent.valid())
		m_listener->SetVelocity(pVelComponent->GetVelocity());

	if(pTrComponent != nullptr) {
		Vector3 forward, up;
		pTrComponent->GetOrientation(&forward, nullptr, &up);
		m_listener->SetOrientation(forward, up);
	}
}

float CListenerComponent::GetGain()
{
	if(m_listener == nullptr)
		return 0.f;
	return m_listener->GetGain();
}

void CListenerComponent::SetGain(float gain)
{
	if(m_listener == nullptr)
		return;
	m_listener->SetGain(gain);
}
void CListenerComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////////////

void CListener::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CListenerComponent>();
}
