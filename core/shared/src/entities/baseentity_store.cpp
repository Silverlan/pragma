/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/baseentity.h"
#include "pragma/entities/entity_component_manager.hpp"
#include "pragma/entities/entity_uuid_ref.hpp"
#include <sharedutils/util_uri.hpp>
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
		SetUuid(util::uuid_string_to_bytes(uuid));

	auto &componentManager = GetNetworkState()->GetGameState()->GetEntityComponentManager();
	auto udmComponents = udm["components"];
	auto numComponents = udmComponents.GetSize();
	for(auto i = decltype(numComponents) {0u}; i < numComponents; ++i) {
		auto udmComponent = udmComponents[i];
		std::string type;
		udmComponent["type"](type);
		auto componentId = pragma::INVALID_COMPONENT_ID;
		if(componentManager.GetComponentTypeId(type, componentId)) {
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
	auto udmComponents = udm.AddArray("components", components.size());
	uint32_t idx = 0;
	// Note: Only one component per type supported!
	for(auto &ptrComponent : components) {
		if(ptrComponent.expired())
			continue;
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
	auto udmData = udm::Data::Create(PSAVE_IDENTIFIER, PSAVE_VERSION);
	auto assetData = udmData->GetAssetData();
	Save(assetData);
	ent->Load(assetData);
	return ent;
}

bool BaseEntity::CreateMemberReference(pragma::EntityIdentifier identifier, std::string var, pragma::EntityUComponentMemberRef &outRef)
{
	if(var.empty()) {
		outRef = pragma::EntityUComponentMemberRef {std::move(identifier), "", ""};
		return true;
	}
	util::Path path {std::move(var)};
	size_t offset = 0;
	if(path.GetComponent(offset, &offset) != "ec") {
		outRef = pragma::EntityUComponentMemberRef {std::move(identifier), "", ""};
		return true;
	}
	auto componentName = path.GetComponent(offset, &offset);
	if(offset == std::string::npos)
		return false;
	auto memberName = path.GetString().substr(offset);
	outRef = pragma::EntityUComponentMemberRef {std::move(identifier), std::string {componentName}, std::string {memberName}};
	return true;
}
bool BaseEntity::ParseUri(std::string uriPath, pragma::EntityUComponentMemberRef &outRef, const util::Uuid *optSelf)
{
	uriparser::Uri uri {std::move(uriPath)};
	auto scheme = uri.scheme();
	if(!scheme.empty() && uri.scheme() != "pragma")
		return false;
	auto uriPathNorm = uri.path();
	ustring::replace(uriPathNorm, "%20", " ");
	util::Path path {uriPathNorm};
	size_t offset = 0;
	if(path.GetComponent(offset, &offset) != "game" || path.GetComponent(offset, &offset) != "entity")
		return false;
	auto queryStr = uri.query();
	std::unordered_map<std::string_view, std::string_view> query;
	uriparser::parse_uri_query(queryStr, query);

	auto &str = path.GetString();
	auto memberName = (offset < str.size()) ? str.substr(offset) : std::string {};

	util::Uuid uuid {};
	auto itUuid = query.find("entity_uuid");
	if(itUuid != query.end())
		return CreateMemberReference(util::uuid_string_to_bytes(std::string {itUuid->second}), std::move(memberName), outRef);
	auto itName = query.find("entity_name");
	if(itName != query.end())
		return CreateMemberReference(std::string {itName->second}, std::move(memberName), outRef);
	if(optSelf)
		return CreateMemberReference(*optSelf, std::move(memberName), outRef);
	return false;
}
std::string BaseEntity::GetUri() const { return GetUri(GetUuid()); }
std::string BaseEntity::GetUri(util::Uuid uuid) { return "pragma:game/entity?entity_uuid=" + util::uuid_to_string(uuid); }
std::string BaseEntity::GetUri(const std::string name) { return "pragma:game/entity?entity_name=" + name; }
