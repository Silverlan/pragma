#include "stdafx_server.h"
#include "pragma/entities/s_npc_dragonworm.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/entities/components/base_model_component.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(npc_dragonworm,NPCDragonWorm);

void SDragonWormComponent::Initialize()
{
	BaseEntityComponent::Initialize();
}

void SDragonWormComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto mdlComponent = GetEntity().GetModelComponent();
	if(mdlComponent.valid())
		mdlComponent->SetModel("creatures/dragonworm.wmd");
}
