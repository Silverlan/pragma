/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
#include "pragma/debug/intel_vtune.hpp"
#include "pragma/asset_types/world.hpp"

pragma::BaseWorldComponent *Game::GetWorld() { return !m_worldComponents.empty() ? m_worldComponents[0].get() : nullptr; }
const std::vector<util::TWeakSharedHandle<pragma::BaseWorldComponent>> &Game::GetWorldComponents() const { return m_worldComponents; }
unsigned int Game::GetEntityCount() { return 0; }
void Game::RemoveEntity(BaseEntity *) {}
BaseEntity *Game::CreateEntity() { return NULL; }
BaseEntity *Game::CreateEntity(std::string classname) { return NULL; }
void Game::SpawnEntity(BaseEntity *ent)
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("spawn_entity");
	util::ScopeGuard sgVtune {[]() { debug::get_domain().EndTask(); }};
#endif
	ent->OnSpawn();
}
BaseEntity *Game::GetEntity(unsigned int) { return NULL; }
BaseEntity *Game::GetEntityByLocalIndex(uint32_t idx) { return GetEntity(idx); }
BaseEntity *Game::FindEntityByUniqueId(const util::Uuid &uuid)
{
	auto it = m_uuidToEnt.find(util::get_uuid_hash(uuid));
	return (it != m_uuidToEnt.end()) ? it->second : nullptr;
}

BaseEntity *Game::CreateMapEntity(pragma::asset::EntityData &entData)
{
	auto *ent = CreateEntity(entData.GetClassName());
	if(ent == nullptr)
		return nullptr;
	auto pTrComponent = ent->GetTransformComponent();
	if(pTrComponent != nullptr)
		pTrComponent->SetPosition(entData.GetOrigin());

	for(auto &componentName : entData.GetComponents())
		ent->AddComponent(componentName);

	auto pMapComponent = ent->AddComponent<pragma::MapComponent>();
	if(pMapComponent.valid())
		pMapComponent->SetMapIndex(entData.GetMapIndex());

	for(auto &pair : entData.GetKeyValues())
		ent->SetKeyValue(pair.first, pair.second);

	auto *pIoComponent = static_cast<pragma::BaseIOComponent *>(ent->FindComponent("io").get());
	if(pIoComponent != nullptr) {
		for(auto &output : entData.GetOutputs())
			pIoComponent->StoreOutput(output.name, output.target, output.input, output.param, output.delay, output.times);
	}

	return ent;
}
