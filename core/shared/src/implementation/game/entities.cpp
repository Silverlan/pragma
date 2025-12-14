// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :game.game;

pragma::BaseWorldComponent *pragma::Game::GetWorld() { return !m_worldComponents.empty() ? m_worldComponents[0].get() : nullptr; }
const std::vector<pragma::util::TWeakSharedHandle<pragma::BaseWorldComponent>> &pragma::Game::GetWorldComponents() const { return m_worldComponents; }
unsigned int pragma::Game::GetEntityCount() { return 0; }
void pragma::Game::RemoveEntity(ecs::BaseEntity *) {}
pragma::ecs::BaseEntity *pragma::Game::CreateEntity() { return nullptr; }
pragma::ecs::BaseEntity *pragma::Game::CreateEntity(std::string classname) { return nullptr; }
void pragma::Game::SpawnEntity(ecs::BaseEntity *ent)
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask("spawn_entity");
	pragma::util::ScopeGuard sgVtune {[]() { ::debug::get_domain().EndTask(); }};
#endif
	ent->OnSpawn();
}
pragma::ecs::BaseEntity *pragma::Game::GetEntity(unsigned int) { return nullptr; }
pragma::ecs::BaseEntity *pragma::Game::GetEntityByLocalIndex(uint32_t idx) { return GetEntity(idx); }
pragma::ecs::BaseEntity *pragma::Game::FindEntityByUniqueId(const util::Uuid &uuid)
{
	auto it = m_uuidToEnt.find(util::get_uuid_hash(uuid));
	return (it != m_uuidToEnt.end()) ? it->second : nullptr;
}

pragma::BaseEntityComponent *pragma::Game::CreateMapComponent(ecs::BaseEntity &ent, const std::string &componentType, const asset::ComponentData &componentData)
{
	auto c = ent.AddComponent(componentType);
	if(c.expired())
		return nullptr;
	auto udmData = componentData.GetData();
	for(auto &pair : udm::LinkedPropertyWrapper {*udmData}.ElIt()) {
		auto *memberInfo = c->FindMemberInfo(std::string {pair.key});
		if(!memberInfo || !memberInfo->setterFunction || !pragma::ents::is_udm_member_type(memberInfo->type))
			continue;
		auto udmType = pragma::ents::member_type_to_udm_type(memberInfo->type);
		auto &udmProp = pair.property;
		auto propType = udmProp.GetType();
		if(!udm::is_convertible(propType, udmType))
			continue;
		udm::visit(propType, [udmType, &udmProp, memberInfo, &c](auto tag) {
			using T0 = typename decltype(tag)::type;
			udm::visit(udmType, [&udmProp, memberInfo, &c](auto tag) {
				using T1 = typename decltype(tag)::type;
				if constexpr(udm::is_convertible<T0, T1>()) {
					auto val = udmProp.ToValue<T1>();
					if(val)
						memberInfo->setterFunction(*memberInfo, *c, &*val);
				}
			});
		});
	}
	return c.get();
}
pragma::ecs::BaseEntity *pragma::Game::CreateMapEntity(asset::EntityData &entData)
{
	auto flags = entData.GetFlags();
	if(math::is_flag_set(flags, asset::EntityData::Flags::ClientsideOnly) && !IsClient())
		return nullptr;
	auto *ent = CreateEntity(entData.GetClassName());
	if(ent == nullptr)
		return nullptr;
	auto pTrComponent = ent->GetTransformComponent();
	if(pTrComponent != nullptr)
		pTrComponent->SetPose(entData.GetEffectivePose());

	for(auto &[componentType, componentData] : entData.GetComponents()) {
		auto flags = componentData->GetFlags();
		if(math::is_flag_set(flags, asset::ComponentData::Flags::ClientsideOnly) && !IsClient())
			continue;
		CreateMapComponent(*ent, componentType, *componentData);
	}

	auto pMapComponent = ent->AddComponent<MapComponent>();
	if(pMapComponent.valid())
		pMapComponent->SetMapIndex(entData.GetMapIndex());

	for(auto &pair : entData.GetKeyValues())
		ent->SetKeyValue(pair.first, pair.second);

	auto *pIoComponent = static_cast<BaseIOComponent *>(ent->FindComponent("io").get());
	if(pIoComponent != nullptr) {
		for(auto &output : entData.GetOutputs())
			pIoComponent->StoreOutput(output.name, output.target, output.input, output.param, output.delay, output.times);
	}

	return ent;
}
