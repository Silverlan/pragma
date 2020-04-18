#include "stdafx_shared.h"
#include <pragma/game/game.h>
#include "pragma/level/mapgeometry.h"
#include "pragma/entities/components/map_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/baseworld.h"
#include "pragma/entities/output.h"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/asset_types/world.hpp"


pragma::BaseWorldComponent *Game::GetWorld() {return m_worldComponent.get();}
unsigned int Game::GetEntityCount() {return 0;}
void Game::RemoveEntity(BaseEntity*) {}
BaseEntity *Game::CreateEntity() {return NULL;}
BaseEntity *Game::CreateEntity(std::string classname) {return NULL;}
void Game::SpawnEntity(BaseEntity *ent) {ent->OnSpawn();}
BaseEntity *Game::GetEntity(unsigned int) {return NULL;}
BaseEntity *Game::GetEntityByLocalIndex(uint32_t idx) {return GetEntity(idx);}

BaseEntity *Game::CreateMapEntity(pragma::asset::EntityData &entData)
{
	auto *ent = CreateEntity(entData.GetClassName());
	if(ent == nullptr)
		return nullptr;
	auto pTrComponent = ent->GetTransformComponent();
	if(pTrComponent.valid())
		pTrComponent->SetPosition(entData.GetOrigin());

	for(auto &componentName : entData.GetComponents())
		ent->AddComponent(componentName);

	auto pMapComponent = ent->AddComponent<pragma::MapComponent>();
	if(pMapComponent.valid())
		pMapComponent->SetMapIndex(entData.GetMapIndex());

	for(auto &pair : entData.GetKeyValues())
		ent->SetKeyValue(pair.first,pair.second);

	auto *pIoComponent = static_cast<pragma::BaseIOComponent*>(ent->FindComponent("io").get());
	if(pIoComponent != nullptr)
	{
		for(auto &output : entData.GetOutputs())
			pIoComponent->StoreOutput(output.name,output.target,output.input,output.param,output.delay,output.times);
	}

	return ent;
}

