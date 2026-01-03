// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.light_map_receiver;
import :client_state;
import :engine;
import :model;

using namespace pragma;

void CLightMapReceiverComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember) { using T = CLightMapReceiverComponent; }
void CLightMapReceiverComponent::SetupLightMapUvData(ecs::CBaseEntity &ent, rendering::LightmapDataCache *cache)
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
	math::set_flag(m_stateFlags, StateFlags::IsModelBakedWithLightMaps, true);
	m_uvDataPerMesh.clear();
	m_meshes.clear();
	m_meshToMeshIdx.clear();
	m_meshToBufIdx.clear();
	math::set_flag(m_stateFlags, StateFlags::RenderMeshBufferIndexTableDirty);
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
		m_meshToMeshIdx.insert(std::make_pair(static_cast<geometry::CModelSubMesh *>(subMesh.get()), subMeshIdx));
		++subMeshIdx;
	}
	UpdateRenderMeshBufferList();
}
const rendering::LightmapDataCache *CLightMapReceiverComponent::GetLightmapDataCache() const { return m_lightmapDataCache.get(); }
void CLightMapReceiverComponent::SetLightmapDataCache(rendering::LightmapDataCache *cache) { m_lightmapDataCache = cache ? cache->shared_from_this() : nullptr; }
const std::vector<Vector2> *CLightMapReceiverComponent::FindLightmapUvSet(geometry::ModelSubMesh &mesh) const
{
	auto *cache = GetLightmapDataCache();
	if(cache)
		return cache->FindLightmapUvs(GetEntity().GetUuid(), mesh.GetUuid());
	return mesh.GetUVSet("lightmap");
}
void CLightMapReceiverComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CLightMapReceiverComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(cModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		//m_isModelBakedWithLightMaps = (GetEntity().GetModelName() == m_modelName); // TODO
		//if(m_isModelBakedWithLightMaps)
		m_meshBufferIndices.clear();
		UpdateModelMeshes();
	});
	BindEventUnhandled(cModelComponent::EVENT_ON_RENDER_MESHES_UPDATED, [this](std::reference_wrapper<ComponentEvent> evData) { UpdateRenderMeshBufferList(); });
	BindEventUnhandled(cBaseEntity::EVENT_ON_SPAWN, [this](std::reference_wrapper<ComponentEvent> evData) {
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
	math::set_flag(m_stateFlags, StateFlags::RenderMeshBufferIndexTableDirty, false);
	auto &renderMeshes = static_cast<CModelComponent *>(mdlC)->GetRenderMeshes();
	m_meshBufferIndices.resize(renderMeshes.size());
	for(auto i = decltype(renderMeshes.size()) {0u}; i < renderMeshes.size(); ++i) {
		auto bufIdx = FindBufferIndex(static_cast<geometry::CModelSubMesh &>(*renderMeshes[i]));
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
	math::set_flag(m_stateFlags, StateFlags::RenderMeshBufferIndexTableDirty);
	m_meshToBufIdx.clear();
	m_meshToMeshIdx.clear();
	uint32_t subMeshIdx = 0u;
	std::vector<std::shared_ptr<geometry::ModelMesh>> meshes;
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
				m_meshToBufIdx.insert(std::make_pair(static_cast<geometry::CModelSubMesh *>(subMesh.get()), itBufIdx->second));
				m_meshToMeshIdx.insert(std::make_pair(static_cast<geometry::CModelSubMesh *>(subMesh.get()), subMeshIdx));
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
	m_meshToBufIdx.insert(std::make_pair(static_cast<geometry::CModelSubMesh *>(itMesh->second.get()), bufIdx));
	math::set_flag(m_stateFlags, StateFlags::RenderMeshBufferIndexTableDirty);
}
std::optional<CLightMapReceiverComponent::BufferIdx> CLightMapReceiverComponent::GetBufferIndex(RenderMeshIndex meshIdx) const
{
	if(math::is_flag_set(m_stateFlags, StateFlags::RenderMeshBufferIndexTableDirty))
		const_cast<CLightMapReceiverComponent *>(this)->UpdateRenderMeshBufferList();
	return (meshIdx < m_meshBufferIndices.size() && m_meshBufferIndices[meshIdx] != std::numeric_limits<BufferIdx>::max()) ? m_meshBufferIndices[meshIdx] : std::optional<BufferIdx> {};
}
std::optional<CLightMapReceiverComponent::BufferIdx> CLightMapReceiverComponent::FindBufferIndex(geometry::CModelSubMesh &mesh) const
{
	if(math::is_flag_set(m_stateFlags, StateFlags::IsModelBakedWithLightMaps) == false)
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
	std::vector<std::shared_ptr<geometry::ModelMesh>> meshes;
	mdlC->GetBaseModelMeshes(meshes);
	for(auto &mesh : meshes) {
		for(auto &subMesh : mesh->GetSubMeshes()) {
			auto bufIdx = FindBufferIndex(*static_cast<geometry::CModelSubMesh *>(subMesh.get()));
			if(bufIdx.has_value() == false)
				continue;
			auto *pUvBuffer = lightMapC.GetMeshLightMapUvBuffer(*bufIdx);
			prosper::IBuffer *pLightMapUvBuffer = nullptr;
			if(pUvBuffer != nullptr)
				pLightMapUvBuffer = pUvBuffer;
			else
				pLightMapUvBuffer = get_cengine()->GetRenderContext().GetDummyBuffer().get();
			mdlC->SetLightmapUvBuffer(*static_cast<geometry::CModelSubMesh *>(subMesh.get()), pLightMapUvBuffer->shared_from_this());
		}
	}
}
