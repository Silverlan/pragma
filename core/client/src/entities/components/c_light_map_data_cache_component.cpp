/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_light_map_data_cache_component.hpp"
#include "pragma/entities/components/c_light_map_receiver_component.hpp"
#include "pragma/entities/components/c_light_map_component.hpp"
#include "pragma/entities/components/lightmap_data_cache.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/logging.hpp"
#include <pragma/entities/components/base_model_component.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_manager_t.hpp>

extern DLLCLIENT CGame *c_game;

using namespace pragma;

void CLightMapDataCacheComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = CLightMapDataCacheComponent;

	using TDataCache = std::string;
	{
		auto memberInfo = create_component_member_info<T, TDataCache, static_cast<void (T::*)(const TDataCache &)>(&T::SetLightMapDataCachePath), static_cast<const TDataCache &(T::*)() const>(&T::GetLightMapDataCachePath)>("lightmapDataCache", "", AttributeSpecializationType::File);
		memberInfo.SetSpecializationType(pragma::AttributeSpecializationType::File);
		auto &metaData = memberInfo.AddMetaData();
		metaData["extensions"] = std::vector<std::string> {pragma::LightmapDataCache::FORMAT_MODEL_BINARY, pragma::LightmapDataCache::FORMAT_MODEL_ASCII};
		metaData["stripRootPath"] = false;
		metaData["stripExtension"] = false;
		registerMember(std::move(memberInfo));
	}
}
CLightMapDataCacheComponent::CLightMapDataCacheComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void CLightMapDataCacheComponent::Initialize() { BaseEntityComponent::Initialize(); }
void CLightMapDataCacheComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CLightMapDataCacheComponent::SetLightMapDataCachePath(const std::string &cachePath)
{
	m_lightmapDataCacheFile = cachePath;
	m_lightmapDataCacheDirty = true;
	SetTickPolicy(pragma::TickPolicy::Always);
}
const std::string &CLightMapDataCacheComponent::GetLightMapDataCachePath() const { return m_lightmapDataCacheFile; }
const std::shared_ptr<LightmapDataCache> &CLightMapDataCacheComponent::GetLightMapDataCache() const { return m_lightmapDataCache; }

void CLightMapDataCacheComponent::OnTick(double dt)
{
	SetTickPolicy(pragma::TickPolicy::Never);
	if(m_lightmapDataCacheDirty) {
		m_lightmapDataCacheDirty = false;
		ReloadCache();
	}
}

void CLightMapDataCacheComponent::InitializeUvBuffers()
{
	if(!m_lightmapDataCache)
		return;
	CLightMapComponent::LOGGER.info("Initializing lightmap uv buffers from cache for {} entities...", m_lightmapDataCache->cacheData.size());
	uint32_t numInitialized = 0;
	for(auto &pair : m_lightmapDataCache->cacheData) {
		EntityIterator entIt {*c_game};
		entIt.AttachFilter<EntityIteratorFilterUuid>(pair.first.uuid);
		auto it = entIt.begin();
		if(it == entIt.end())
			continue;
		pragma::CLightMapReceiverComponent::SetupLightMapUvData(static_cast<CBaseEntity &>(**it), m_lightmapDataCache.get());
		++numInitialized;
	}

	if(numInitialized == 0)
		CLightMapComponent::LOGGER.warn("No entities found for lightmap data cache!");

	// Generate lightmap uv buffers for all entities
	std::vector<std::shared_ptr<prosper::IBuffer>> buffers {};
	auto globalLightmapUvBuffer = pragma::CLightMapComponent::GenerateLightmapUVBuffers(buffers);

	if(globalLightmapUvBuffer) {
		EntityIterator entIt {*c_game};
		entIt.AttachFilter<EntityIteratorFilterUuid>(m_lightmapDataCache->lightmapEntityId);
		auto it = entIt.begin();
		if(it != entIt.end()) {
			auto *ent = *it;
			assert(ent);
			auto lightMapC = ent->GetComponent<pragma::CLightMapComponent>();
			if(lightMapC.valid()) {
				// lightMapC->SetLightMapIntensity(worldData.GetLightMapIntensity());
				// lightMapC->SetLightMapExposure(worldData.GetLightMapExposure());
				lightMapC->InitializeLightMapData(nullptr, globalLightmapUvBuffer, buffers, nullptr, true);
				auto *scene = c_game->GetRenderScene();
				if(scene)
					scene->SetLightMap(*lightMapC);
				else
					CLightMapComponent::LOGGER.error("Failed to set lightmap on render scene!");
			}
			else
				CLightMapComponent::LOGGER.error("Failed to find lightmap component on entity with ID {}!", util::uuid_to_string(m_lightmapDataCache->lightmapEntityId));
		}
		else
			CLightMapComponent::LOGGER.error("Failed to find lightmap entity with ID {}!", util::uuid_to_string(m_lightmapDataCache->lightmapEntityId));
	}
	else
		CLightMapComponent::LOGGER.error("Failed to initialize global lightmap uv buffer!");
}
void CLightMapDataCacheComponent::ReloadCache()
{
	CLightMapComponent::LOGGER.info("Reloading lightmap data cache from cache file '{}'...", m_lightmapDataCacheFile);
	m_lightmapDataCache = std::make_shared<LightmapDataCache>();
	std::string err;
	if(!LightmapDataCache::Load(m_lightmapDataCacheFile, *m_lightmapDataCache, err))
		m_lightmapDataCache = nullptr;

	if(!m_lightmapDataCache) {
		CLightMapComponent::LOGGER.error("Failed to load lightmap data cache: {}", err);
		return;
	}
	std::unordered_map<std::string, std::shared_ptr<Model>> cachedModels;
	for(auto &pair : m_lightmapDataCache->cacheData) {
		EntityIterator entIt {*c_game, EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
		entIt.AttachFilter<EntityIteratorFilterUuid>(pair.first.uuid);

		auto it = entIt.begin();
		if(it == entIt.end()) {
			CLightMapComponent::LOGGER.warn("Entity '{}' defined in lightmap data cache does not exist!", util::uuid_to_string(pair.first.uuid));
			continue;
		}
		auto *ent = *it;
		if(!ent) {
			CLightMapComponent::LOGGER.warn("Entity '{}' defined in lightmap data cache is invalid!", util::uuid_to_string(pair.first.uuid));
			continue;
		}
		auto &mdl = ent->GetModel();
		if(!mdl) {
			CLightMapComponent::LOGGER.warn("Entity '{}' defined in lightmap data cache has no valid model!", util::uuid_to_string(pair.first.uuid));
			continue;
		}
		auto mdlName = mdl->GetName();
		if(mdlName != pair.second.model) {
			CLightMapComponent::LOGGER.warn("Model name '{}' of entity '{}' defined in lightmap data cache does not match expected model '{}'!", mdlName, util::uuid_to_string(pair.first.uuid), pair.second.model);
			continue;
		}
		auto itCache = cachedModels.find(mdlName);
		if(itCache != cachedModels.end() && itCache->second == nullptr) {
			CLightMapComponent::LOGGER.warn("Invalid model cache for entity '{}' defined in lightmap data cache! Ignoring...", util::uuid_to_string(pair.first.uuid));
			continue;
		}
		auto hasLightmapData = false;
		if(itCache == cachedModels.end()) {
			for(auto &mg : mdl->GetMeshGroups()) {
				for(auto &mesh : mg->GetMeshes()) {
					for(auto &subMesh : mesh->GetSubMeshes()) {
						if(subMesh->GetExtensionData()["lightmapData"]) {
							hasLightmapData = true;
							goto endLoop;
						}
					}
				}
			}
		endLoop:;
		}
		else
			hasLightmapData = (itCache->second != nullptr);

		std::shared_ptr<Model> lmModel = nullptr;
		if(itCache != cachedModels.end())
			lmModel = itCache->second;
		else if(!hasLightmapData) {
			CLightMapComponent::LOGGER.warn("Model '{}' of entity '{}' defined in lightmap data cache does not have lightmap uv data!", mdlName, util::uuid_to_string(pair.first.uuid));
			cachedModels[mdlName] = nullptr;
		}
		else {
			auto cpy = mdl->Copy(GetEntity().GetNetworkState()->GetGameState(), Model::CopyFlags::CopyMeshesBit | Model::CopyFlags::CopyUniqueIdsBit);
			for(auto &mg : cpy->GetMeshGroups()) {
				for(auto &mesh : mg->GetMeshes()) {
					auto &subMeshes = mesh->GetSubMeshes();
					auto n = subMeshes.size();
					for(auto i = decltype(n) {0u}; i < n; ++i) {
						auto &subMesh = subMeshes[i];
						auto extData = subMesh->GetExtensionData();
						auto udmLightmapData = extData["lightmapData"];
						if(udmLightmapData) {
							auto udmMeshData = udmLightmapData["meshData"];
							std::vector<umath::Vertex> verts;
							udmMeshData["vertices"](verts);

							subMesh->GetVertices() = std::move(verts);

							auto udmIndices = udmMeshData["indices"];
							auto *a = udmIndices.GetValuePtr<udm::Array>();
							if(a) {
								udm::visit<true, false, false>(a->GetValueType(), [&udmMeshData, &subMesh](auto tag) {
									using T = typename decltype(tag)::type;
									if constexpr(std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t>) {
										std::vector<T> indices;
										udmMeshData["indices"](indices);
										subMesh->SetIndices(indices);
									}
								});
							}
						}
					}
				}
			}
			cpy->Update(ModelUpdateFlags::UpdatePrimitiveCounts | ModelUpdateFlags::UpdateBuffers | ModelUpdateFlags::UpdateChildren);
			cachedModels[cpy->GetName()] = cpy;
			lmModel = cpy;
		}
		if(lmModel) {
			uint32_t skin = 0;
			std::vector<uint32_t> bgs;
			auto *mdlC = ent->GetModelComponent();
			if(mdlC) {
				skin = mdlC->GetSkin();
				bgs = mdlC->GetBodyGroups();
			}
			ent->SetModel(lmModel);
			mdlC = ent->GetModelComponent();
			if(mdlC) {
				mdlC->SetSkin(skin);
				for(auto i = decltype(bgs.size()) {0u}; i < bgs.size(); ++i)
					mdlC->SetBodyGroup(i, bgs[i]);
			}
		}

		auto lc = ent->AddComponent<CLightMapReceiverComponent>();
		assert(lc.valid());
		if(lc.valid())
			lc->SetLightmapDataCache(m_lightmapDataCache.get());
	}
	InitializeUvBuffers();
}
