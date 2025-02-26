/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/c_light_map_receiver_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_light_map_component.hpp"
#include "pragma/entities/components/lightmap_data_cache.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/entity_component_manager_t.hpp>
#include <buffers/prosper_buffer.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/logging.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

void CLightMapReceiverComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember) { using T = CLightMapReceiverComponent; }
void CLightMapReceiverComponent::SetupLightMapUvData(CBaseEntity &ent, LightmapDataCache *cache)
{
	auto mdl = ent.GetModel();
	if(!mdl) {
		CLightMapComponent::LOGGER.warn("Unable to find lightmap uv data for entity '{}': Entity has no model!", ent.ToString());
		return;
	}
	auto hasLightmapUvs = false;
	for(auto &mg : mdl->GetMeshGroups()) {
		for(auto &mesh : mg->GetMeshes()) {
			for(auto &subMesh : mesh->GetSubMeshes()) {
				const std::vector<Vector2> *uvSet = nullptr;
				if(cache)
					uvSet = cache->FindLightmapUvs(ent.GetUuid(), subMesh->GetUuid());
				else
					uvSet = subMesh->GetUVSet("lightmap");
				if(uvSet) {
					hasLightmapUvs = true;
					goto endLoop;
				}
			}
		}
	}
endLoop:
	if(hasLightmapUvs) {
		auto lightMapReceiverC = ent.AddComponent<CLightMapReceiverComponent>();
		if(lightMapReceiverC.valid())
			lightMapReceiverC->UpdateLightMapUvData();
	}
}
void CLightMapReceiverComponent::UpdateLightMapUvData()
{
	auto mdlC = GetEntity().GetComponent<CModelComponent>();
	auto mdl = GetEntity().GetModel();
	auto renderC = GetEntity().GetComponent<CRenderComponent>();
	if(mdlC.expired() || renderC.expired() || mdl == nullptr) {
		CLightMapComponent::LOGGER.warn("Unable to initialize lightmap uv data for entity '{}': Entity has no model!", GetEntity().ToString());
		return;
	}
	m_modelName = mdl->GetName();
	umath::set_flag(m_stateFlags, StateFlags::IsModelBakedWithLightMaps, true);
	m_uvDataPerMesh.clear();
	m_meshes.clear();
	m_meshToMeshIdx.clear();
	m_meshToBufIdx.clear();
	umath::set_flag(m_stateFlags, StateFlags::RenderMeshBufferIndexTableDirty);
	uint32_t subMeshIdx = 0u;
	auto wasInitialized = false;
	for(auto &subMesh : renderC->GetRenderMeshes()) {
		auto *uvSet = FindLightmapUvSet(*subMesh);
		if(uvSet == nullptr) {
			++subMeshIdx;
			continue;
		}
		m_uvDataPerMesh.insert(std::make_pair(subMeshIdx, *uvSet));
		m_meshes.insert(std::make_pair(subMeshIdx, subMesh));
		m_meshToMeshIdx.insert(std::make_pair(static_cast<CModelSubMesh *>(subMesh.get()), subMeshIdx));
		++subMeshIdx;
	}
	UpdateRenderMeshBufferList();
}
const LightmapDataCache *CLightMapReceiverComponent::GetLightmapDataCache() const { return m_lightmapDataCache.get(); }
void CLightMapReceiverComponent::SetLightmapDataCache(LightmapDataCache *cache) { m_lightmapDataCache = cache ? cache->shared_from_this() : nullptr; }
const std::vector<Vector2> *CLightMapReceiverComponent::FindLightmapUvSet(ModelSubMesh &mesh) const
{
	auto *cache = GetLightmapDataCache();
	if(cache)
		return cache->FindLightmapUvs(GetEntity().GetUuid(), mesh.GetUuid());
	return mesh.GetUVSet("lightmap");
}
void CLightMapReceiverComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CLightMapReceiverComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(CModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		//m_isModelBakedWithLightMaps = (GetEntity().GetModelName() == m_modelName); // TODO
		//if(m_isModelBakedWithLightMaps)
		m_meshBufferIndices.clear();
		UpdateModelMeshes();
	});
	BindEventUnhandled(CModelComponent::EVENT_ON_RENDER_MESHES_UPDATED, [this](std::reference_wrapper<ComponentEvent> evData) { UpdateRenderMeshBufferList(); });
	BindEventUnhandled(CBaseEntity::EVENT_ON_SPAWN, [this](std::reference_wrapper<ComponentEvent> evData) {
		//m_isModelBakedWithLightMaps = (GetEntity().GetModelName() == m_modelName); // TODO
		//if(m_isModelBakedWithLightMaps)
		UpdateModelMeshes();
	});
	if(GetEntity().IsSpawned()) {
		UpdateModelMeshes();
		UpdateRenderMeshBufferList();
	}
}
void CLightMapReceiverComponent::UpdateRenderMeshBufferList()
{
	m_meshBufferIndices.clear();
	auto mdlC = GetEntity().GetModelComponent();
	if(!mdlC) {
		CLightMapComponent::LOGGER.warn("Unable to update render mesh buffer list: No model component found for entity '{}'!", GetEntity().ToString());
		return;
	}
	umath::set_flag(m_stateFlags, StateFlags::RenderMeshBufferIndexTableDirty, false);
	auto &renderMeshes = static_cast<CModelComponent *>(mdlC)->GetRenderMeshes();
	m_meshBufferIndices.resize(renderMeshes.size());
	for(auto i = decltype(renderMeshes.size()) {0u}; i < renderMeshes.size(); ++i) {
		auto bufIdx = FindBufferIndex(static_cast<CModelSubMesh &>(*renderMeshes[i]));
		m_meshBufferIndices[i] = bufIdx.has_value() ? *bufIdx : std::numeric_limits<BufferIdx>::max();
	}
}
void CLightMapReceiverComponent::UpdateModelMeshes()
{
	m_meshes.clear();
	auto mdlC = GetEntity().GetComponent<CModelComponent>();
	auto mdl = GetEntity().GetModel();
	if(mdlC.expired() || mdl == nullptr) {
		CLightMapComponent::LOGGER.warn("Unable to update model meshes: No model component found for entity '{}'!", GetEntity().ToString());
		return;
	}
	auto renderC = GetEntity().GetComponent<CRenderComponent>();
	if(renderC.expired()) {
		CLightMapComponent::LOGGER.warn("Unable to update model meshes: No render component found for entity '{}'!", GetEntity().ToString());
		return;
	}
	std::unordered_map<MeshIdx, BufferIdx> meshIdxToBufIdx {};
	for(auto &pair : m_meshToMeshIdx) {
		auto *mesh = pair.first;
		auto it = m_meshToBufIdx.find(mesh);
		if(it == m_meshToBufIdx.end())
			continue;
		meshIdxToBufIdx.insert(std::make_pair(pair.second, it->second));
	}
	umath::set_flag(m_stateFlags, StateFlags::RenderMeshBufferIndexTableDirty);
	m_meshToBufIdx.clear();
	m_meshToMeshIdx.clear();
	uint32_t subMeshIdx = 0u;
	std::vector<std::shared_ptr<ModelMesh>> meshes;
	mdlC->GetBaseModelMeshes(meshes);
	for(auto &mesh : meshes) {
		for(auto &subMesh : mesh->GetSubMeshes()) {
			auto *uvSet = FindLightmapUvSet(*subMesh);
			if(uvSet == nullptr) {
				++subMeshIdx;
				continue;
			}
			m_meshes.insert(std::make_pair(subMeshIdx, subMesh));
			auto itBufIdx = meshIdxToBufIdx.find(subMeshIdx);
			if(itBufIdx != meshIdxToBufIdx.end()) {
				m_meshToBufIdx.insert(std::make_pair(static_cast<CModelSubMesh *>(subMesh.get()), itBufIdx->second));
				m_meshToMeshIdx.insert(std::make_pair(static_cast<CModelSubMesh *>(subMesh.get()), subMeshIdx));
			}
			++subMeshIdx;
		}
	}
}
const std::unordered_map<CLightMapReceiverComponent::MeshIdx, std::vector<Vector2>> &CLightMapReceiverComponent::GetMeshLightMapUvData() const { return m_uvDataPerMesh; }
void CLightMapReceiverComponent::AssignBufferIndex(MeshIdx meshIdx, BufferIdx bufIdx)
{
	auto itMesh = m_meshes.find(meshIdx);
	if(itMesh == m_meshes.end()) {
		CLightMapComponent::LOGGER.warn("Unable to assign buffer index for buffer {} of mesh {} to lightmap receiver component of entity {}: Mesh not found!", bufIdx, meshIdx, GetEntity().ToString());
		return;
	}
	m_meshToBufIdx.insert(std::make_pair(static_cast<CModelSubMesh *>(itMesh->second.get()), bufIdx));
	umath::set_flag(m_stateFlags, StateFlags::RenderMeshBufferIndexTableDirty);
}
std::optional<pragma::CLightMapReceiverComponent::BufferIdx> CLightMapReceiverComponent::GetBufferIndex(RenderMeshIndex meshIdx) const
{
	if(umath::is_flag_set(m_stateFlags, StateFlags::RenderMeshBufferIndexTableDirty))
		const_cast<CLightMapReceiverComponent *>(this)->UpdateRenderMeshBufferList();
	return (meshIdx < m_meshBufferIndices.size() && m_meshBufferIndices[meshIdx] != std::numeric_limits<BufferIdx>::max()) ? m_meshBufferIndices[meshIdx] : std::optional<pragma::CLightMapReceiverComponent::BufferIdx> {};
}
std::optional<CLightMapReceiverComponent::BufferIdx> CLightMapReceiverComponent::FindBufferIndex(CModelSubMesh &mesh) const
{
	if(umath::is_flag_set(m_stateFlags, StateFlags::IsModelBakedWithLightMaps) == false)
		return {};
	auto it = m_meshToBufIdx.find(&mesh);
	if(it == m_meshToBufIdx.end())
		return {};
	return it->second;
}
void CLightMapReceiverComponent::UpdateMeshLightmapUvBuffers(CLightMapComponent &lightMapC)
{
	auto mdlC = GetEntity().GetComponent<CModelComponent>();
	auto mdl = GetEntity().GetModel();
	if(mdlC.expired() || mdl == nullptr) {
		CLightMapComponent::LOGGER.warn("Unable to update mesh lightmap uv buffers: No model component found for entity '{}'!", GetEntity().ToString());
		return;
	}
	uint32_t subMeshIdx = 0u;
	std::vector<std::shared_ptr<ModelMesh>> meshes;
	mdlC->GetBaseModelMeshes(meshes);
	for(auto &mesh : meshes) {
		for(auto &subMesh : mesh->GetSubMeshes()) {
			auto bufIdx = FindBufferIndex(*static_cast<CModelSubMesh *>(subMesh.get()));
			if(bufIdx.has_value() == false)
				continue;
			auto *pUvBuffer = lightMapC.GetMeshLightMapUvBuffer(*bufIdx);
			prosper::IBuffer *pLightMapUvBuffer = nullptr;
			if(pUvBuffer != nullptr)
				pLightMapUvBuffer = pUvBuffer;
			else
				pLightMapUvBuffer = c_engine->GetRenderContext().GetDummyBuffer().get();
			mdlC->SetLightmapUvBuffer(*static_cast<CModelSubMesh *>(subMesh.get()), pLightMapUvBuffer->shared_from_this());
		}
	}
}
