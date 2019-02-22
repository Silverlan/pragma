#include "stdafx_shared.h"
#include "pragma/entities/baseentity.h"
#include "pragma/entities/entity_component_manager.hpp"
#include <sharedutils/datastream.h>

static const auto ENTITY_DATA_VERSION = 1u;

void BaseEntity::Load(DataStream &ds)
{
	auto version = ds->Read<uint32_t>();

	auto spawnFlags = ds->Read<uint32_t>();
	SetSpawnFlags(m_spawnFlags);

	auto &componentManager = GetNetworkState()->GetGameState()->GetEntityComponentManager();
	auto numComponents = ds->Read<uint32_t>();
	for(auto i=decltype(numComponents){0u};i<numComponents;++i)
	{
		auto szComponent = ds->Read<uint32_t>();
		auto offset = ds->GetOffset();
		auto componentName = ds->ReadString();
		auto idx = ds->Read<uint16_t>();
		auto componentId = pragma::INVALID_COMPONENT_ID;
		componentManager.GetComponentTypeId(componentName,componentId);
		util::WeakHandle<pragma::BaseEntityComponent> whComponent = {};
		if(idx == 0u)
			whComponent = AddComponent(componentName);
		else
		{
			// TODO: This is terrible, improve this!
			for(auto &component : GetComponents())
			{
				if(component->GetComponentId() != componentId || --idx > 0u)
					continue;
				whComponent = component;
				break;
			}
			while(idx-- > 0u)
				whComponent = AddComponent(componentName);
		}
		if(whComponent.valid())
			whComponent->Load(ds);
		ds->SetOffset(offset +szComponent);
	}

	// Networked variables
	// TODO
}
void BaseEntity::Save(DataStream &ds)
{
	ds->Write<uint32_t>(ENTITY_DATA_VERSION);
	ds->Write<uint32_t>(m_spawnFlags);

	auto &componentManager = GetNetworkState()->GetGameState()->GetEntityComponentManager();
	auto &components = GetComponents();
	ds->Write<uint32_t>(components.size());
	std::unordered_map<pragma::ComponentId,uint16_t> componentCounts {};
	for(auto &ptrComponent : components)
	{
		auto *pComponentInfo = componentManager.GetComponentInfo(ptrComponent->GetComponentId());
		auto offsetToComponentSize = ds->GetOffset();
		ds->Write<uint32_t>(0u);
		auto componentId = ptrComponent->GetComponentId();
		auto it = componentCounts.find(componentId);
		if(it == componentCounts.end())
			it = componentCounts.insert(std::make_pair(componentId,0u)).first;
		ds->WriteString(pComponentInfo->name);
		ds->Write<uint16_t>(it->second++); // The nth component of this type
		ptrComponent->Save(ds);
		auto szComponent = ds->GetOffset() -(offsetToComponentSize +sizeof(uint32_t));
		ds->Write<uint32_t>(szComponent,&offsetToComponentSize);
	}

	// Networked variables
	// TODO
}
BaseEntity *BaseEntity::Copy()
{
	auto *nw = GetNetworkState();
	auto *game = nw->GetGameState();
	auto *ent = game->CreateEntity(GetClass());
	if(ent == nullptr)
		return nullptr;
	DataStream entData {};
	Save(entData);
	entData->SetOffset(0);

	ent->Load(entData);
	return ent;
}
