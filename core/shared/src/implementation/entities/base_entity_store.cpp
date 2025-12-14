// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.base_entity;

static const auto ENTITY_DATA_VERSION = 1u;

void pragma::ecs::BaseEntity::Load(udm::LinkedPropertyWrapper &udm)
{
	uint32_t spawnFlags = m_spawnFlags;
	udm["spawnFlags"] >> spawnFlags;
	SetSpawnFlags(spawnFlags);
	std::string uuid;
	udm["uuid"] >> uuid;
	if(util::is_uuid(uuid))
		SetUuid(util::uuid_string_to_bytes(uuid));

	auto &componentManager = GetNetworkState()->GetGameState()->GetEntityComponentManager();
	auto udmComponents = udm["components"];
	auto numComponents = udmComponents.GetSize();
	for(auto i = decltype(numComponents) {0u}; i < numComponents; ++i) {
		auto udmComponent = udmComponents[i];
		std::string type;
		udmComponent["type"] >> type;
		auto componentId = INVALID_COMPONENT_ID;
		if(componentManager.GetComponentTypeId(type, componentId)) {
			auto hComponent = AddComponent(componentId);
			if(hComponent.valid())
				hComponent->Load(udmComponent);
		}
	}

	// Networked variables
	// TODO
}
void pragma::ecs::BaseEntity::Save(udm::LinkedPropertyWrapper &udm)
{
	udm["entityDataVersion"] << ENTITY_DATA_VERSION;
	udm["spawnFlags"] << m_spawnFlags;
	udm["uuid"] << util::uuid_to_string(m_uuid);

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
		udmComponent["type"] << pComponentInfo->name.str;
		ptrComponent->Save(udmComponent);
	}

	// Networked variables
	// TODO
}
pragma::ecs::BaseEntity *pragma::ecs::BaseEntity::Copy()
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

bool pragma::ecs::BaseEntity::CreateMemberReference(EntityIdentifier identifier, std::string var, EntityUComponentMemberRef &outRef)
{
	if(var.empty()) {
		outRef = EntityUComponentMemberRef {std::move(identifier), "", ""};
		return true;
	}
	auto path = util::RelFile(std::move(var));
	size_t offset = 0;
	if(path.GetComponent(offset, &offset) != "ec") {
		outRef = EntityUComponentMemberRef {std::move(identifier), "", ""};
		return true;
	}
	auto componentName = path.GetComponent(offset, &offset);
	if(offset == std::string::npos)
		return false;
	auto memberName = path.GetString().substr(offset);
	outRef = EntityUComponentMemberRef {std::move(identifier), std::string {componentName}, std::string {memberName}};
	return true;
}
bool pragma::ecs::BaseEntity::ParseUri(std::string uriPath, EntityUComponentMemberRef &outRef, const util::Uuid *optSelf)
{
	uriparser::Uri uri {std::move(uriPath)};
	auto scheme = uri.scheme();
	if(!scheme.empty() && uri.scheme() != "pragma")
		return false;
	auto uriPathNorm = uri.path();
	string::replace(uriPathNorm, "%20", " ");
	auto path = util::RelFile(uriPathNorm);
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
std::string pragma::ecs::BaseEntity::GetUri() const { return GetUri(GetUuid()); }
std::string pragma::ecs::BaseEntity::GetUri(util::Uuid uuid) { return "pragma:game/entity?entity_uuid=" + util::uuid_to_string(uuid); }
std::string pragma::ecs::BaseEntity::GetUri(const std::string name) { return "pragma:game/entity?entity_name=" + name; }
