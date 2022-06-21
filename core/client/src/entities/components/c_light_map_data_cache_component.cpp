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
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_manager_t.hpp>

extern DLLCLIENT CGame *c_game;

using namespace pragma;
#pragma optimize("",off)
void CLightMapDataCacheComponent::RegisterMembers(pragma::EntityComponentManager &componentManager,TRegisterComponentMember registerMember)
{
	using T = CLightMapDataCacheComponent;

	using TDataCache = std::string;
	{
		auto memberInfo = create_component_member_info<
			T,TDataCache,
			static_cast<void(T::*)(const TDataCache&)>(&T::SetLightMapDataCachePath),
			static_cast<const TDataCache&(T::*)() const>(&T::GetLightMapDataCachePath)
		>("lightmapDataCache","");
		registerMember(std::move(memberInfo));
	}
}
CLightMapDataCacheComponent::CLightMapDataCacheComponent(BaseEntity &ent)
	: BaseEntityComponent(ent)
{}
void CLightMapDataCacheComponent::Initialize()
{
	BaseEntityComponent::Initialize();
}
void CLightMapDataCacheComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l);}

void CLightMapDataCacheComponent::SetLightMapDataCachePath(const std::string &cachePath) {m_lightmapDataCacheFile = cachePath;}
const std::string &CLightMapDataCacheComponent::GetLightMapDataCachePath() const {return m_lightmapDataCacheFile;}
const std::shared_ptr<LightmapDataCache> &CLightMapDataCacheComponent::GetLightMapDataCache() const {return m_lightmapDataCache;}

void CLightMapDataCacheComponent::InitializeUvBuffers()
{
	if(!m_lightmapDataCache)
		return;
	for(auto &pair : m_lightmapDataCache->cacheData)
	{
		EntityIterator entIt {*c_game};
		entIt.AttachFilter<EntityIteratorFilterUuid>(pair.first.uuid);
		auto it = entIt.begin();
		if(it == entIt.end())
			continue;
		pragma::CLightMapReceiverComponent::SetupLightMapUvData(static_cast<CBaseEntity&>(**it),m_lightmapDataCache.get());
	}

	// Generate lightmap uv buffers for all entities
	std::vector<std::shared_ptr<prosper::IBuffer>> buffers {};
	auto globalLightmapUvBuffer = pragma::CLightMapComponent::GenerateLightmapUVBuffers(buffers);

	if(globalLightmapUvBuffer)
	{
		EntityIterator entIt {*c_game};
		entIt.AttachFilter<EntityIteratorFilterUuid>(m_lightmapDataCache->lightmapEntityId);
		auto it = entIt.begin();
		if(it != entIt.end())
		{
			auto *ent = *it;
			assert(ent);
			auto lightMapC = ent->GetComponent<pragma::CLightMapComponent>();
			if(lightMapC.valid())
			{
				// lightMapC->SetLightMapIntensity(worldData.GetLightMapIntensity());
				// lightMapC->SetLightMapExposure(worldData.GetLightMapExposure());
				lightMapC->InitializeLightMapData(nullptr,globalLightmapUvBuffer,buffers,nullptr,true);
				auto *scene = c_game->GetRenderScene();
				if(scene)
					scene->SetLightMap(*lightMapC);
			}
		}
	}
}
void CLightMapDataCacheComponent::ReloadCache()
{
	m_lightmapDataCache = std::make_shared<LightmapDataCache>();
	std::string err;
	if(!LightmapDataCache::Load(m_lightmapDataCacheFile,*m_lightmapDataCache,err))
		m_lightmapDataCache = nullptr;

	if(!m_lightmapDataCache)
		return;
	std::unordered_map<std::string,std::shared_ptr<Model>> cachedModels;
	for(auto &pair : m_lightmapDataCache->cacheData)
	{
		EntityIterator entIt {*c_game};
		entIt.AttachFilter<TEntityIteratorFilterComponent<CLightMapReceiverComponent>>();
		entIt.AttachFilter<EntityIteratorFilterUuid>(pair.first.uuid);

		auto it = entIt.begin();
		if(it == entIt.end())
			continue;
		auto *ent = *it;
		if(!ent)
			continue;
		auto &mdl = ent->GetModel();
		if(!mdl || mdl->GetName() != pair.second.model)
			continue;
		auto itCache = cachedModels.find(mdl->GetName());
		if(itCache != cachedModels.end() && itCache->second == nullptr)
			continue;
		auto hasLightmapData = false;
		if(itCache == cachedModels.end())
		{
			for(auto &mg : mdl->GetMeshGroups())
			{
				for(auto &mesh : mg->GetMeshes())
				{
					for(auto &subMesh : mesh->GetSubMeshes())
					{
						if(subMesh->GetExtensionData()["lightmapData"])
						{
							hasLightmapData = true;
							goto endLoop;
						}
					}
				}
			}
		endLoop:
			;
		}
		else
			hasLightmapData = (itCache->second != nullptr);

		std::shared_ptr<Model> lmModel = nullptr;
		if(itCache != cachedModels.end())
			lmModel = itCache->second;
		else if(!hasLightmapData)
			cachedModels[mdl->GetName()] = nullptr;
		else
		{
			auto cpy = mdl->Copy(GetEntity().GetNetworkState()->GetGameState(),Model::CopyFlags::CopyMeshesBit | Model::CopyFlags::CopyUniqueIdsBit);
			for(auto &mg : cpy->GetMeshGroups())
			{
				for(auto &mesh : mg->GetMeshes())
				{
					auto &subMeshes = mesh->GetSubMeshes();
					auto n = subMeshes.size();
					for(auto i=decltype(n){0u};i<n;++i)
					{
						auto &subMesh = subMeshes[i];
						auto extData = subMesh->GetExtensionData();
						auto udmLightmapData = extData["lightmapData"];
						if(udmLightmapData)
						{
							auto udmMeshData = udmLightmapData["meshData"];
							std::vector<umath::Vertex> verts;
							udmMeshData["vertices"](verts);

							subMesh->GetVertices() = std::move(verts);

							auto udmIndices = udmMeshData["indices"];
							auto *a = udmIndices.GetValuePtr<udm::Array>();
							if(a)
							{
								udm::visit<true,false,false>(a->GetValueType(),[&udmMeshData,&subMesh](auto tag) {
									using T = decltype(tag)::type;
									if constexpr(std::is_same_v<T,uint16_t> || std::is_same_v<T,uint32_t>)
									{
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
			cpy->Update(ModelUpdateFlags::UpdatePrimitiveCounts | ModelUpdateFlags::UpdateBuffers);
			cachedModels[cpy->GetName()] = cpy;
			lmModel = cpy;
		}
		if(lmModel)
			ent->SetModel(lmModel);

		auto lc = ent->GetComponent<CLightMapReceiverComponent>();
		if(lc.valid())
			lc->SetLightmapDataCache(m_lightmapDataCache.get());
	}
	InitializeUvBuffers();
}
#pragma optimize("",on)
