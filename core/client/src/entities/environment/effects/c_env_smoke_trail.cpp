/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/environment/effects/c_env_smoke_trail.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/util/util_smoke_trail.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/components/basetoggle.h>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_smoke_trail, CEnvSmokeTrail);

extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

CSmokeTrailComponent::~CSmokeTrailComponent() { DestroyParticle(); }
void CSmokeTrailComponent::Initialize()
{
	BaseEnvSmokeTrailComponent::Initialize();
	pragma::CParticleSystemComponent::Precache("smoke.wpt");
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
	m_hParticle = pt->GetHandle<pragma::CParticleSystemComponent>();
}

void CSmokeTrailComponent::DestroyParticle()
{
	if(m_hParticle.valid()) {
		m_hParticle->Die();
		m_hParticle->SetRemoveOnComplete(true);
	}
}
void CSmokeTrailComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////////

void CEnvSmokeTrail::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSmokeTrailComponent>();
}
