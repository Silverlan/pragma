// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"


module pragma.client;


import :entities.components.env_weather;
import :client_state;
import :engine;
import :entities.components.attachment;
import :entities.components.particle_system;
import :entities.components.player;
import :game;

using namespace pragma;


CWeatherComponent::~CWeatherComponent()
{
	if(m_hParticle.valid())
		m_hParticle->Remove();
}
void CWeatherComponent::Initialize()
{
	BaseEnvWeatherComponent::Initialize();
	pragma::ecs::CParticleSystemComponent::Precache("weather");
}
void CWeatherComponent::OnEntitySpawn()
{
	BaseEnvWeatherComponent::OnEntitySpawn();
	auto *pl = pragma::get_cgame()->GetLocalPlayer();
	if(pl != nullptr) {
		auto *pt = pragma::ecs::CParticleSystemComponent::Create({{"particle", "snow"}});
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
				attInfo.flags |= pragma::FAttachmentMode::PositionOnly;
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
