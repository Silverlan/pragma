#include "stdafx_client.h"
#include "pragma/entities/environment/c_env_weather.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/components/base_transform_component.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_weather,CEnvWeather);

extern DLLCLIENT CGame *c_game;

CWeatherComponent::~CWeatherComponent()
{
	if(m_hParticle.IsValid())
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
	if(pl != NULL)
	{
		auto *pt = CParticleSystemComponent::Create({{"particle","snow"}});
		if(pt != nullptr)
		{
			auto &plEnt = pl->GetEntity();
			auto &ent = pt->GetEntity();
			auto pTrComponent = ent.GetTransformComponent();
			auto pTrComponentPl = plEnt.GetTransformComponent();
			if(pTrComponent.valid() && pTrComponentPl.valid())
				pTrComponent->SetPosition(pTrComponentPl->GetPosition());
			auto pAttComponent = ent.AddComponent<CAttachableComponent>();
			if(pAttComponent.valid())
			{
				AttachmentInfo attInfo {};
				attInfo.flags |= FAttachmentMode::PositionOnly;
				pAttComponent->AttachToEntity(&plEnt,attInfo);
			}
			ent.Spawn();
			m_hParticle = ent.GetHandle();
		}
	}
}
luabind::object CWeatherComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CWeatherComponentHandleWrapper>(l);}

/////////////

void CEnvWeather::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CWeatherComponent>();
}
