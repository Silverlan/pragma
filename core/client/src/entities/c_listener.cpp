/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/c_listener.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/entities/components/c_transform_component.hpp"
#include <alsoundsystem.hpp>
#include <alsound_listener.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/components/velocity_component.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;

LINK_ENTITY_TO_CLASS(listener, CListener);

void CListenerComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	auto &ent = GetEntity();
	ent.AddComponent<pragma::CTransformComponent>();
	auto *soundSys = c_engine->GetSoundSystem();
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
	auto pVelComponent = ent.GetComponent<pragma::VelocityComponent>();
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
void CListenerComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////////////

void CListener::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CListenerComponent>();
}
