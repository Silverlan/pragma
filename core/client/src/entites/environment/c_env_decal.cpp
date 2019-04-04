#include "stdafx_client.h"
#include "pragma/entities/environment/c_env_decal.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/physics/raytraces.h>
#include <pragma/entities/environment/env_decal.h>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_decal,CEnvDecal);

extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

void CDecalComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = static_cast<CBaseEntity&>(GetEntity());
	auto pSpriteComponent = ent.AddComponent<pragma::CSpriteComponent>();
	if(pSpriteComponent.valid())
		pSpriteComponent->SetOrientationType(pragma::CParticleSystemComponent::OrientationType::World);
}

void CDecalComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent.expired())
		return;
	auto &pos = pTrComponent->GetPosition();
	auto dir = -pTrComponent->GetForward();

	auto *pSpriteComponent = static_cast<pragma::CSpriteComponent*>(ent.FindComponent("sprite").get());
	TraceData tr {};
	tr.SetSource(pos);
	tr.SetTarget(pos +dir *18.f);
	tr.SetFlags(FTRACE::IGNORE_DYNAMIC);
	auto r = c_game->RayCast(tr);
	if(r.hit == true)
	{
		pTrComponent->SetPosition(r.position +dir *-std::numeric_limits<float>::epsilon());
		auto &rot = pTrComponent->GetOrientation();
		auto angle = uquat::get_angle(rot);
		pTrComponent->SetOrientation(uquat::create(r.normal,angle));
	}
	if(pSpriteComponent != nullptr)
		pSpriteComponent->StartParticle();
}

luabind::object CDecalComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CDecalComponentHandleWrapper>(l);}

//////////////

void CEnvDecal::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CDecalComponent>();
}
