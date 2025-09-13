// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include "pragma/entities/parentmode.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"

module pragma.client.entities.components.env_weather;

import pragma.client.client_state;
import pragma.client.entities.components.attachment;
import pragma.client.entities.components.player;

using namespace pragma;

extern CGame *c_game;

CWeatherComponent::~CWeatherComponent()
{
	if(m_hParticle.valid())
		m_hParticle->Remove();
}
void CWeatherComponent::Initialize()
{
	BaseEnvWeatherComponent::Initialize();
	CParticleSystemComponent::Precache("weather");
}
void CWeatherComponent::OnEntitySpawn()
{
	BaseEnvWeatherComponent::OnEntitySpawn();
	auto *pl = c_game->GetLocalPlayer();
	if(pl != NULL) {
		auto *pt = CParticleSystemComponent::Create({{"particle", "snow"}});
		if(pt != nullptr) {
			auto &plEnt = pl->GetEntity();
			auto &ent = pt->GetEntity();
			auto pTrComponent = ent.GetTransformComponent();
			auto pTrComponentPl = plEnt.GetTransformComponent();
			if(pTrComponent != nullptr && pTrComponentPl)
				pTrComponent->SetPosition(pTrComponentPl->GetPosition());
			auto pAttComponent = ent.AddComponent<CAttachmentComponent>();
			if(pAttComponent.valid()) {
				AttachmentInfo attInfo {};
				attInfo.flags |= FAttachmentMode::PositionOnly;
				pAttComponent->AttachToEntity(&plEnt, attInfo);
			}
			ent.Spawn();
			m_hParticle = ent.GetHandle();
		}
	}
}
void CWeatherComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////

void CEnvWeather::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CWeatherComponent>();
}
