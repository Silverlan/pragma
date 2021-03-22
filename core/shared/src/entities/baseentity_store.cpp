/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/baseentity.h"
#include "pragma/entities/entity_component_manager.hpp"
#include <sharedutils/datastream.h>
#include <udm.hpp>

static const auto ENTITY_DATA_VERSION = 1u;

void BaseEntity::Load(udm::LinkedPropertyWrapper &udm)
{
	uint32_t spawnFlags = m_spawnFlags;
	udm["spawnFlags"](spawnFlags);
	SetSpawnFlags(spawnFlags);
	std::string uuid;
	udm["uuid"](uuid);
	if(util::is_uuid(uuid))
		m_uuid = util::uuid_string_to_bytes(uuid);

	auto &componentManager = GetNetworkState()->GetGameState()->GetEntityComponentManager();
	auto udmComponents = udm["components"];
	auto numComponents = udmComponents.GetSize();
	for(auto i=decltype(numComponents){0u};i<numComponents;++i)
	{
		auto udmComponent = udmComponents[i];
		std::string type;
		udmComponent["type"](type);
		auto componentId = pragma::INVALID_COMPONENT_ID;
		if(componentManager.GetComponentTypeId(type,componentId))
		{
			auto hComponent = AddComponent(componentId);
			if(hComponent.valid())
				hComponent->Load(udmComponent);
		}
	}

	// Networked variables
	// TODO
}
void BaseEntity::Save(udm::LinkedPropertyWrapper &udm)
{
	udm["entityDataVersion"] = ENTITY_DATA_VERSION;
	udm["spawnFlags"] = m_spawnFlags;
	udm["uuid"] = util::uuid_to_string(m_uuid);

	auto &componentManager = GetNetworkState()->GetGameState()->GetEntityComponentManager();
	auto &components = GetComponents();
	auto udmComponents = udm.AddArray("components",components.size());
	uint32_t idx = 0;
	// Note: Also one component per type supported!
	for(auto &ptrComponent : components)
	{
		auto udmComponent = udmComponents[idx++];
		auto *pComponentInfo = componentManager.GetComponentInfo(ptrComponent->GetComponentId());
		udmComponent["type"] = pComponentInfo->name;
		ptrComponent->Save(udmComponent);
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
	auto udmData = udm::Data::Create(PSAVE_IDENTIFIER,PSAVE_VERSION);
	auto assetData = udmData->GetAssetData();
	Save(assetData);
	ent->Load(assetData);
	return ent;
}
