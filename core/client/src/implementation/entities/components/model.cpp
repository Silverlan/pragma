// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :entities.components.model;
import :client_state;
import :engine;
import :game;
import :model;

using namespace pragma;

ComponentEventId cModelComponent::EVENT_ON_RENDER_MESHES_UPDATED = INVALID_COMPONENT_ID;
ComponentEventId cModelComponent::EVENT_ON_GAME_SHADER_SPECIALIZATION_CONSTANT_FLAGS_UPDATED = INVALID_COMPONENT_ID;
void CModelComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CModelComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	BaseModelComponent::RegisterEvents(componentManager, registerEvent);
	cModelComponent::EVENT_ON_RENDER_MESHES_UPDATED = registerEvent("EVENT_ON_RENDER_MESHES_UPDATED", ComponentEventInfo::Type::Explicit);
	cModelComponent::EVENT_ON_GAME_SHADER_SPECIALIZATION_CONSTANT_FLAGS_UPDATED = registerEvent("EVENT_ON_GAME_SHADER_SPECIALIZATION_CONSTANT_FLAGS_UPDATED", ComponentEventInfo::Type::Broadcast);
}

CModelComponent::CModelComponent(ecs::BaseEntity &ent) : BaseModelComponent(ent), m_baseShaderSpecializationConstantFlags {GameShaderSpecializationConstantFlag::None}, m_staticShaderSpecializationConstantFlags {GameShaderSpecializationConstantFlag::None} {}

void CModelComponent::Initialize()
{
	BaseModelComponent::Initialize();
	auto &ent = GetEntity();
	auto pRenderComponent = ent.GetComponent<CRenderComponent>();
	if(pRenderComponent.valid())
		pRenderComponent->SetRenderBufferDirty();
	UpdateBaseShaderSpecializationFlags();

	BindEventUnhandled(cRenderComponent::EVENT_ON_CLIP_PLANE_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) { UpdateBaseShaderSpecializationFlags(); });
	BindEventUnhandled(cRenderComponent::EVENT_ON_DEPTH_BIAS_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) { UpdateBaseShaderSpecializationFlags(); });
	BindEventUnhandled(cColorComponent::EVENT_ON_COLOR_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) { UpdateBaseShaderSpecializationFlags(); });
	BindEventUnhandled(baseModelComponent::EVENT_ON_SKIN_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		math::set_flag(m_stateFlags, StateFlags::RenderBufferListUpdateRequired);
		SetTickPolicy(TickPolicy::Always);
	});
	BindEventUnhandled(baseModelComponent::EVENT_ON_BODY_GROUP_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		math::set_flag(m_stateFlags, StateFlags::RenderMeshUpdateRequired);
		SetTickPolicy(TickPolicy::Always);
	});
}

void CModelComponent::UpdateBaseShaderSpecializationFlags()
{
	auto clipPlane = false;
	auto depthBias = false;
	auto &ent = static_cast<ecs::CBaseEntity &>(GetEntity());
	auto *renderC = ent.GetRenderComponent();
	if(renderC != nullptr) {
		clipPlane = renderC->GetRenderClipPlane();
		depthBias = renderC->GetDepthBias();
	}
	// pragma::math::set_flag(m_baseShaderSpecializationConstantFlags, GameShaderSpecializationConstantFlag::EnableClippingBit, clipPlane);
	// pragma::math::set_flag(m_baseShaderSpecializationConstantFlags, GameShaderSpecializationConstantFlag::EnableDepthBias, depthBias);

	auto colorC = ent.GetComponent<CColorComponent>();
	math::set_flag(m_baseShaderSpecializationConstantFlags, GameShaderSpecializationConstantFlag::EnableTranslucencyBit, colorC.valid() && colorC->GetColor().a < 1.f);

	m_baseShaderSpecializationConstantFlags |= m_staticShaderSpecializationConstantFlags;

	InvokeEventCallbacks(cModelComponent::EVENT_ON_GAME_SHADER_SPECIALIZATION_CONSTANT_FLAGS_UPDATED);
}

CMaterialOverrideComponent *CModelComponent::GetMaterialOverrideComponent() { return m_materialOverrideComponent; }

material::CMaterial *CModelComponent::GetRenderMaterial(uint32_t idx, uint32_t skin) const
{
	// TODO: Move this to Model class
	auto &mdl = GetModel();
	if(mdl == nullptr)
		return nullptr;
	auto *texGroup = mdl->GetTextureGroup(skin);
	if(texGroup == nullptr)
		texGroup = mdl->GetTextureGroup(0); // Fall back to default skin
	if(texGroup == nullptr || idx >= texGroup->textures.size())
		return nullptr;
	idx = texGroup->textures.at(idx);
	if(m_materialOverrideComponent) {
		auto *matOverride = m_materialOverrideComponent->GetRenderMaterial(idx);
		if(matOverride)
			return matOverride;
	}
	auto *mat = static_cast<material::CMaterial *>(mdl->GetMaterial(idx));
	return mat ? mat : static_cast<material::CMaterial *>(get_client_state()->GetMaterialManager().GetErrorMaterial());
}
material::CMaterial *CModelComponent::GetRenderMaterial(uint32_t idx) const { return GetRenderMaterial(idx, GetSkin()); }

Bool CModelComponent::ReceiveNetEvent(NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetBodyGroup) {
		auto groupId = packet->Read<UInt32>();
		auto id = packet->Read<UInt32>();
		SetBodyGroup(groupId, id);
		return true;
	}
	else if(eventId == m_netEvMaxDrawDist) {
		m_maxDrawDistance = packet->Read<float>();
		return true;
	}
	return false;
}

void CModelComponent::ReceiveData(NetPacket &packet)
{
	std::string mdl = packet->ReadString();
	if(!mdl.empty())
		SetModel(mdl.c_str());
	SetSkin(packet->Read<unsigned int>());
	m_maxDrawDistance = packet->Read<float>();

	auto numBodyGroups = packet->Read<uint32_t>();
	for(auto i = decltype(numBodyGroups) {0}; i < numBodyGroups; ++i) {
		auto bg = packet->Read<uint32_t>();
		SetBodyGroup(i, bg);
	}
}

bool CModelComponent::IsWeighted() const
{
	auto animComponent = GetEntity().GetAnimatedComponent();
	return animComponent.valid() && animComponent->GetBoneCount() > 0u && animComponent->GetBindPose();
}

uint32_t CModelComponent::GetLOD() const { return m_lod; }

void CModelComponent::GetBaseModelMeshes(std::vector<std::shared_ptr<geometry::ModelMesh>> &outMeshes, uint32_t lod) const
{
	auto &mdl = GetModel();
	if(mdl == nullptr)
		return;
	mdl->GetBodyGroupMeshes(GetBodyGroups(), lod, outMeshes);
}

const std::shared_ptr<prosper::IRenderBuffer> &CModelComponent::GetRenderBuffer(uint32_t idx) const { return m_lodMeshRenderBufferData[idx].renderBuffer; }
const rendering::RenderBufferData *CModelComponent::GetRenderBufferData(uint32_t idx) const { return &m_lodMeshRenderBufferData[idx]; }
GameShaderSpecializationConstantFlag CModelComponent::GetPipelineSpecializationFlags(uint32_t idx) const { return m_lodMeshRenderBufferData[idx].pipelineSpecializationFlags; }

void CModelComponent::ReloadRenderBufferList(bool immediate)
{
	if(immediate)
		UpdateRenderBufferList();
	else
		math::set_flag(m_stateFlags, StateFlags::RenderBufferListUpdateRequired);
}

void CModelComponent::SetLightmapUvBuffer(const geometry::CModelSubMesh &mesh, const std::shared_ptr<prosper::IBuffer> &buffer) { m_lightmapUvBuffers[&mesh] = buffer; }
std::shared_ptr<prosper::IBuffer> CModelComponent::GetLightmapUvBuffer(const geometry::CModelSubMesh &mesh) const
{
	auto it = m_lightmapUvBuffers.find(&mesh);
	return (it != m_lightmapUvBuffers.end()) ? it->second : nullptr;
}

bool CModelComponent::IsDepthPrepassEnabled() const { return !math::is_flag_set(m_stateFlags, StateFlags::DepthPrepassDisabled); }
void CModelComponent::SetDepthPrepassEnabled(bool enabled) { math::set_flag(m_stateFlags, StateFlags::DepthPrepassDisabled, !enabled); }

void CModelComponent::SetRenderBufferData(const std::vector<rendering::RenderBufferData> &renderBufferData) { m_lodMeshRenderBufferData = renderBufferData; }

void CModelComponent::AddRenderMesh(geometry::CModelSubMesh &mesh, material::CMaterial &mat, rendering::RenderBufferData::StateFlags stateFlags)
{
	if(m_lodRenderMeshGroups.empty())
		return;
	auto *shader = dynamic_cast<ShaderGameWorldLightingPass *>(mat.GetPrimaryShader());
	if(!shader || !shader->IsValid())
		return;
	if(!shader->InitializeMaterialDescriptorSet(mat))
		return;
	auto &renderBuffer = mesh.GetRenderBuffer(*shader);
	if(!renderBuffer)
		return;
	auto &lodGroup = m_lodRenderMeshGroups.front();
	auto insertIdx = lodGroup.first + lodGroup.second;

	if(!shader || !shader->IsDepthPrepassEnabled())
		math::set_flag(stateFlags, rendering::RenderBufferData::StateFlags::EnableDepthPrepass, false);

	rendering::RenderBufferData renderBufferData {};
	renderBufferData.material = mat.GetHandle();
	renderBufferData.renderBuffer = renderBuffer;
	renderBufferData.stateFlags = stateFlags;
	renderBufferData.pipelineSpecializationFlags = shader->GetMaterialPipelineSpecializationRequirements(mat);

	m_lodRenderMeshes.insert(m_lodRenderMeshes.begin() + insertIdx, mesh.shared_from_this());
	m_lodMeshRenderBufferData.insert(m_lodMeshRenderBufferData.begin() + insertIdx, std::move(renderBufferData));

	++lodGroup.second;
	for(auto i = decltype(m_lodRenderMeshGroups.size()) {1u}; i < m_lodRenderMeshGroups.size(); ++i)
		++m_lodRenderMeshGroups[i].first;
}

void CModelComponent::UpdateRenderBufferList()
{
	if(std::this_thread::get_id() != get_cengine()->GetMainThreadId()) {
		Con::CWAR << "Attempted to update render meshes from non-main thread, this is illegal!" << Con::endl;
		return;
	}
	math::set_flag(m_stateFlags, StateFlags::RenderBufferListUpdateRequired, false);
	for(auto &bufData : m_lodMeshRenderBufferData)
		get_cengine()->GetRenderContext().KeepResourceAliveUntilPresentationComplete(bufData.renderBuffer);
	m_lodMeshRenderBufferData.clear();
	m_lodMeshRenderBufferData.reserve(m_lodRenderMeshes.size());
	auto depthPrepassEnabled = IsDepthPrepassEnabled();
	get_cengine()->GetRenderContext().GetPipelineLoader().Flush();
	for(auto i = decltype(m_lodRenderMeshes.size()) {0u}; i < m_lodRenderMeshes.size(); ++i) {
		auto &mesh = static_cast<geometry::CModelSubMesh &>(*m_lodRenderMeshes[i]);
		auto *mat = GetRenderMaterial(mesh.GetSkinTextureIndex());
		std::shared_ptr<prosper::IRenderBuffer> renderBuffer = nullptr;
		auto *shader = mat ? dynamic_cast<ShaderGameWorldLightingPass *>(mat->GetPrimaryShader()) : nullptr;
		if(shader && shader->IsValid()) {
			renderBuffer = mesh.GetRenderBuffer(*shader);

			auto uvBuffer = GetLightmapUvBuffer(mesh);
			if(uvBuffer) {
				auto &buffers = renderBuffer->GetBuffers();
				std::vector<prosper::IBuffer *> newBuffers;
				newBuffers.reserve(buffers.size());
				for(auto &buffer : buffers)
					newBuffers.push_back(buffer.get());
				auto *indexBuffer = renderBuffer->GetIndexBufferInfo();
				auto lightmapUvIndex = math::to_integral(ShaderGameWorldLightingPass::VertexBinding::LightmapUv);
				if(lightmapUvIndex < newBuffers.size()) {
					newBuffers[lightmapUvIndex] = uvBuffer.get();
					renderBuffer = get_cengine()->GetRenderContext().CreateRenderBuffer(renderBuffer->GetPipelineCreateInfo(), newBuffers, renderBuffer->GetOffsets(), indexBuffer ? std::optional<prosper::IndexBufferInfo> {*indexBuffer} : std::optional<prosper::IndexBufferInfo> {});
				}
			}

#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
			::debug::get_domain().BeginTask("init_mat_desc_set");
#endif
			if(!shader->InitializeMaterialDescriptorSet(*mat))
				mat = nullptr;
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
			::debug::get_domain().EndTask();
#endif
		}
		m_lodMeshRenderBufferData.push_back({});
		auto &renderBufferData = m_lodMeshRenderBufferData.back();
		renderBufferData.renderBuffer = renderBuffer;
		renderBufferData.material = mat ? mat->GetHandle() : material::MaterialHandle {};
		math::set_flag(renderBufferData.stateFlags, rendering::RenderBufferData::StateFlags::EnableDepthPrepass, depthPrepassEnabled && shader && shader->IsDepthPrepassEnabled());
		if(mat == nullptr || shader == nullptr)
			continue;
		renderBufferData.pipelineSpecializationFlags = shader->GetMaterialPipelineSpecializationRequirements(*mat);
		if(mat->GetProperty("test_glow", false))
			math::set_flag(renderBufferData.stateFlags, rendering::RenderBufferData::StateFlags::EnableGlowPass);
	}
}

void CModelComponent::SetBaseShaderSpecializationFlag(GameShaderSpecializationConstantFlag flag, bool enabled) { math::set_flag(m_baseShaderSpecializationConstantFlags, flag, enabled); }

void CModelComponent::UpdateRenderMeshes(bool requireBoundingVolumeUpdate)
{
	if(math::is_flag_set(m_stateFlags, StateFlags::RenderMeshUpdateRequired | StateFlags::RenderBufferListUpdateRequired) == false)
		return;
	if(std::this_thread::get_id() != get_cengine()->GetMainThreadId()) {
		Con::CWAR << "Attempted to update render meshes from non-main thread, this is illegal!" << Con::endl;
		return;
	}
	auto renderMeshesUpdated = false;
	if(math::is_flag_set(m_stateFlags, StateFlags::RenderMeshUpdateRequired)) {
		math::set_flag(m_stateFlags, StateFlags::RenderMeshUpdateRequired, false);
		m_lodRenderMeshes.clear();
		m_lodMeshes.clear();
		m_lodMeshGroups.clear();
		m_lodRenderMeshGroups.clear();

		auto &mdl = GetModel();
		auto numLods = math::max(mdl ? mdl->GetLODCount() : 1u, static_cast<uint32_t>(1));
		m_lodRenderMeshGroups.resize(numLods);
		m_lodMeshGroups.resize(numLods);
		if(mdl != nullptr) {
			for(auto i = decltype(numLods) {0u}; i < numLods; ++i) {
				auto meshOffset = m_lodMeshes.size();
				auto subMeshOffset = m_lodRenderMeshes.size();
				mdl->GetBodyGroupMeshes(GetBodyGroups(), i, m_lodMeshes);
				for(auto i = meshOffset; i < m_lodMeshes.size(); ++i) {
					auto &mesh = m_lodMeshes[i];
					for(auto &subMesh : mesh->GetSubMeshes()) {
						if(m_lodRenderMeshes.size() == m_lodRenderMeshes.capacity())
							m_lodRenderMeshes.reserve(m_lodRenderMeshes.size() * 1.4 + 10);
						m_lodRenderMeshes.push_back(subMesh);
					}
				}
				m_lodMeshGroups[i] = {meshOffset, m_lodMeshes.size() - meshOffset};
				m_lodRenderMeshGroups[i] = {subMeshOffset, m_lodRenderMeshes.size() - subMeshOffset};
			}
		}

		renderMeshesUpdated = true;
	}
	UpdateRenderBufferList();
	if(renderMeshesUpdated)
		BroadcastEvent(cModelComponent::EVENT_ON_RENDER_MESHES_UPDATED, CEOnRenderMeshesUpdated {requireBoundingVolumeUpdate});
}

void CModelComponent::UpdateLOD(UInt32 lod)
{
	m_lod = lod;
	UpdateRenderMeshes();
	//std::unordered_map<unsigned int,RenderInstance*>::iterator it = m_renderInstances.find(m_lod);
	//if(it != m_renderInstances.end())
	//	it->second->SetEnabled(false);
	//CUChar(lod);

	//UpdateRenderMeshes();
	//it = m_renderInstances.find(m_lod);
	//if(it != m_renderInstances.end())
	//	it->second->SetEnabled(true);
}

void CModelComponent::SetLOD(uint32_t lod) { m_lod = lod; }

void CModelComponent::SetAutoLodEnabled(bool enabled) { math::set_flag(m_stateFlags, StateFlags::AutoLodDisabled, !enabled); }
bool CModelComponent::IsAutoLodEnabled() const { return !math::is_flag_set(m_stateFlags, StateFlags::AutoLodDisabled); }

void CModelComponent::UpdateLOD(const CSceneComponent &scene, const CCameraComponent &cam, const Mat4 &vp)
{
	UpdateRenderMeshes();
	if(IsAutoLodEnabled() == false)
		return;
	auto &mdl = GetModel();
	if(mdl == nullptr)
		return;
	auto numLods = mdl->GetLODCount();
	if(numLods <= 1 && m_maxDrawDistance == 0.f)
		return;
	auto t = get_cgame()->CurTime();
	if(t < m_tNextLodUpdate)
		return;
	// Updating LODs is relatively expensive, but there's really no reason to update them every frame,
	// so we'll only update them once a while.
	// We'll use random time intervals to lower the risk of a bunch of objects updating at the same time.
	// TODO: This doesn't really work if the object is being rendered from multiple perspectives with different distances in the same frame!
	// Also, changing LODs should occur via a fade-effect (both meshes could be added to render queue with alpha modifier)
	m_tNextLodUpdate = t + math::random(0.2f, 0.6f);

	auto &pos = GetEntity().GetPosition();
	auto d = uvec::distance(pos, cam.GetEntity().GetPosition());
	constexpr auto LOD_CAMERA_DISTANCE_THRESHOLD = 20.f;
	if(math::abs(d - m_lastLodCamDistance) < LOD_CAMERA_DISTANCE_THRESHOLD)
		return; // Don't bother updating if the distance to the camera hasn't changed much. TODO: This also doesn't work well with different perspectives in the same frame!

	if(m_maxDrawDistance > 0.f && d >= m_maxDrawDistance) {
		UpdateLOD(std::numeric_limits<uint32_t>::max());
		return;
	}
	m_lastLodCamDistance = d;

	if(numLods <= 1) {
		UpdateLOD(0);
		return;
	}

	// TODO: This needs optimizing
	auto w = scene.GetWidth();
	auto h = scene.GetHeight();
	auto posOffset = pos + cam.GetEntity().GetUp() * 1.f;
	auto uvMin = umat::to_screen_uv(pos, vp);
	auto uvMax = umat::to_screen_uv(posOffset, vp);
	auto extents = math::max(uvMin.y, uvMax.y) - math::min(uvMin.y, uvMax.y);
	extents *= h;
	extents *= 2.f; // TODO: Why?

	auto size = 100.f / extents;
	auto &lods = mdl->GetLODs();
	uint32_t lod = 0;
	for(auto i = decltype(lods.size()) {0u}; i < lods.size(); ++i) {
		auto &lodInfo = lods.at(i);
		if(size >= lodInfo.distance) {
			lod = i + 1;
			continue;
		}
		break;
	}
	lod += get_cgame()->GetLODBias();
	if(m_lod == lod)
		return;
	UpdateLOD(lod);
}

std::vector<std::shared_ptr<geometry::ModelMesh>> &CModelComponent::GetLODMeshes() { return m_lodMeshes; }
const std::vector<std::shared_ptr<geometry::ModelMesh>> &CModelComponent::GetLODMeshes() const { return const_cast<CModelComponent *>(this)->GetLODMeshes(); }
std::vector<std::shared_ptr<geometry::ModelSubMesh>> &CModelComponent::GetRenderMeshes() { return m_lodRenderMeshes; }
const std::vector<std::shared_ptr<geometry::ModelSubMesh>> &CModelComponent::GetRenderMeshes() const { return const_cast<CModelComponent *>(this)->GetRenderMeshes(); }

rendering::RenderMeshGroup &CModelComponent::GetLodMeshGroup(uint32_t lod)
{
	if(m_lod == std::numeric_limits<uint32_t>::max()) {
		static rendering::RenderMeshGroup emptyGroup {};
		return emptyGroup; // TODO: This should only be returned as const!
	}
	UpdateRenderMeshes();
	lod = math::min(lod, static_cast<uint32_t>(m_lodMeshGroups.size() - 1));
	assert(lod < m_lodMeshGroups.size());
	return m_lodMeshGroups[lod];
}
const rendering::RenderMeshGroup &CModelComponent::GetLodMeshGroup(uint32_t lod) const { return const_cast<CModelComponent *>(this)->GetLodMeshGroup(lod); }
rendering::RenderMeshGroup &CModelComponent::GetLodRenderMeshGroup(uint32_t lod)
{
	if(m_lod == std::numeric_limits<uint32_t>::max()) {
		static rendering::RenderMeshGroup emptyGroup {};
		return emptyGroup; // TODO: This should only be returned as const!
	}
	UpdateRenderMeshes();
	lod = math::min(lod, static_cast<uint32_t>(m_lodRenderMeshGroups.size() - 1));
	assert(lod < m_lodRenderMeshGroups.size());
	if(lod >= m_lodRenderMeshGroups.size()) {
		static rendering::RenderMeshGroup emptyGroup {};
		return emptyGroup; // TODO: This should only be returned as const!
	}
	return m_lodRenderMeshGroups[lod];
}
const rendering::RenderMeshGroup &CModelComponent::GetLodRenderMeshGroup(uint32_t lod) const { return const_cast<CModelComponent *>(this)->GetLodRenderMeshGroup(lod); }

void CModelComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseModelComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CBvhComponent))
		m_bvhComponent = static_cast<CBvhComponent *>(&component);
	else if(typeid(component) == typeid(CMaterialOverrideComponent)) {
		m_materialOverrideComponent = static_cast<CMaterialOverrideComponent *>(&component);
		math::set_flag(m_stateFlags, StateFlags::RenderMeshUpdateRequired, true);
		SetTickPolicy(TickPolicy::Always);
	}
}
void CModelComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseModelComponent::OnEntityComponentRemoved(component);
	if(typeid(component) == typeid(CBvhComponent))
		m_bvhComponent = nullptr;
	else if(typeid(component) == typeid(CMaterialOverrideComponent)) {
		m_materialOverrideComponent = nullptr;
		math::set_flag(m_stateFlags, StateFlags::RenderMeshUpdateRequired, true);
		SetTickPolicy(TickPolicy::Always);
	}
}

void CModelComponent::FlushRenderData()
{
	if(math::is_flag_set(m_stateFlags, StateFlags::RenderMeshUpdateRequired))
		UpdateRenderMeshes();
	if(math::is_flag_set(m_stateFlags, StateFlags::RenderBufferListUpdateRequired))
		UpdateRenderBufferList();
}

void CModelComponent::SetRenderBufferListUpdateRequired() { math::set_flag(m_stateFlags, StateFlags::RenderBufferListUpdateRequired); }

void CModelComponent::OnTick(double tDelta)
{
	FlushRenderData();
	SetTickPolicy(TickPolicy::Never);
}

bool CModelComponent::SetBodyGroup(UInt32 groupId, UInt32 id)
{
	auto r = BaseModelComponent::SetBodyGroup(groupId, id);
	if(r == false)
		return r;
	math::set_flag(m_stateFlags, StateFlags::RenderMeshUpdateRequired);
	SetTickPolicy(TickPolicy::Always);
	// UpdateLOD(m_lod); // Update our active meshes
	return true;
}

void CModelComponent::SetRenderMeshesDirty() { math::set_flag(m_stateFlags, StateFlags::RenderMeshUpdateRequired); }

void CModelComponent::OnModelChanged(const std::shared_ptr<asset::Model> &model)
{
	auto &ent = GetEntity();
	auto pRenderComponent = ent.GetComponent<CRenderComponent>();
	if(pRenderComponent.valid())
		pRenderComponent->SetRenderBufferDirty();

	m_lod = 0;

	m_lodRenderMeshes.clear();
	m_lodMeshes.clear();

	m_lodMeshGroups.clear();
	m_lodMeshGroups.push_back({0, 0});
	m_lodRenderMeshGroups.clear();
	m_lodRenderMeshGroups.push_back({0, 0});

	m_lightmapUvBuffers.clear();

	SetRenderMeshesDirty();

	if(model == nullptr) {
		BaseModelComponent::OnModelChanged(model);
		return;
	}
	UpdateLOD(0);
	for(auto &hMat : model->GetMaterials()) {
		if(!hMat)
			continue;
		hMat->UpdateTextures(); // Ensure all textures have been fully loaded
	}
	BaseModelComponent::OnModelChanged(model);
}

////////////

CEOnRenderMeshesUpdated::CEOnRenderMeshesUpdated(bool requireBoundingVolumeUpdate) : requireBoundingVolumeUpdate {requireBoundingVolumeUpdate} {}
void CEOnRenderMeshesUpdated::PushArguments(lua::State *l) { Lua::PushBool(l, requireBoundingVolumeUpdate); }

void CModelComponent::RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts)
{
	BaseModelComponent::RegisterLuaBindings(l, modEnts);
	auto defCModel = pragma::LuaCore::create_entity_component_class<CModelComponent, BaseModelComponent>("ModelComponent");
	defCModel.add_static_constant("EVENT_ON_RENDER_MESHES_UPDATED", cModelComponent::EVENT_ON_RENDER_MESHES_UPDATED);
	defCModel.add_static_constant("EVENT_ON_GAME_SHADER_SPECIALIZATION_CONSTANT_FLAGS_UPDATED", cModelComponent::EVENT_ON_GAME_SHADER_SPECIALIZATION_CONSTANT_FLAGS_UPDATED);
	//Lua::register_base_model_component_methods<luabind::class_<CModelHandle,BaseEntityComponentHandle>,CModelHandle>(l,defCModel);
	defCModel.def("GetRenderMaterial", static_cast<material::CMaterial *(CModelComponent::*)(uint32_t, uint32_t) const>(&CModelComponent::GetRenderMaterial));
	defCModel.def("GetRenderMaterial", static_cast<material::CMaterial *(CModelComponent::*)(uint32_t) const>(&CModelComponent::GetRenderMaterial));
	defCModel.def("GetLOD", &CModelComponent::GetLOD);
	defCModel.def("IsAutoLodEnabled", &CModelComponent::IsAutoLodEnabled);
	defCModel.def("SetAutoLodEnabled", &CModelComponent::SetAutoLodEnabled);
	defCModel.def("SetMaxDrawDistance", &CModelComponent::SetMaxDrawDistance);
	defCModel.def("GetMaxDrawDistance", &CModelComponent::GetMaxDrawDistance);
	defCModel.def("UpdateRenderMeshes", &CModelComponent::UpdateRenderMeshes, luabind::default_parameter_policy<2, true> {});
	defCModel.def("UpdateRenderMeshes", &CModelComponent::UpdateRenderMeshes);
	defCModel.def("UpdateRenderBufferList", &CModelComponent::UpdateRenderBufferList);
	defCModel.def("SetRenderMeshesDirty", &CModelComponent::SetRenderMeshesDirty);
	defCModel.def("ReloadRenderBufferList", &CModelComponent::ReloadRenderBufferList, luabind::default_parameter_policy<2, false> {});
	defCModel.def("ReloadRenderBufferList", &CModelComponent::ReloadRenderBufferList);
	defCModel.def("IsDepthPrepassEnabled", &CModelComponent::IsDepthPrepassEnabled);
	defCModel.def("SetDepthPrepassEnabled", &CModelComponent::SetDepthPrepassEnabled);
	defCModel.def("SetRenderBufferData", &CModelComponent::SetRenderBufferData);
	defCModel.def("GetRenderBufferData", +[](CModelComponent &c) -> std::vector<rendering::RenderBufferData> { return c.GetRenderBufferData(); });
	defCModel.def("AddRenderMesh", &CModelComponent::AddRenderMesh);
	defCModel.def("AddRenderMesh", &CModelComponent::AddRenderMesh, luabind::default_parameter_policy<4, rendering::RenderBufferData::StateFlags::EnableDepthPrepass> {});
	defCModel.def("GetRenderMeshes", +[](CModelComponent &c) -> std::vector<std::shared_ptr<geometry::ModelSubMesh>> { return c.GetRenderMeshes(); });
	defCModel.def("GetBaseShaderSpecializationFlags", &CModelComponent::GetBaseShaderSpecializationFlags);
	defCModel.def("SetBaseShaderSpecializationFlags", &CModelComponent::SetBaseShaderSpecializationFlags);
	defCModel.def("SetBaseShaderSpecializationFlag", &CModelComponent::SetBaseShaderSpecializationFlag);
	defCModel.def("SetBaseShaderSpecializationFlag", +[](CModelComponent &c, GameShaderSpecializationConstantFlag flag) { c.SetBaseShaderSpecializationFlag(flag); });
	defCModel.def("GetStaticShaderSpecializationFlags", &CModelComponent::GetStaticShaderSpecializationFlags);
	defCModel.def("SetStaticShaderSpecializationFlags", &CModelComponent::SetStaticShaderSpecializationFlags);
	defCModel.def("SetStaticShaderSpecializationFlags", &CModelComponent::SetStaticShaderSpecializationFlags);

	auto defRenderBufferData = luabind::class_<rendering::RenderBufferData>("RenderBufferData");
	defRenderBufferData.add_static_constant("STATE_FLAG_NONE", math::to_integral(rendering::RenderBufferData::StateFlags::None));
	defRenderBufferData.add_static_constant("STATE_FLAG_ENABLE_DEPTH_PREPASS_BIT", math::to_integral(rendering::RenderBufferData::StateFlags::EnableDepthPrepass));
	defRenderBufferData.add_static_constant("STATE_FLAG_ENABLE_GLOW_PASS_BIT", math::to_integral(rendering::RenderBufferData::StateFlags::EnableGlowPass));
	defRenderBufferData.add_static_constant("STATE_FLAG_EXCLUDE_FROM_ACCELERATION_STRUCTURES_BIT", math::to_integral(rendering::RenderBufferData::StateFlags::ExcludeFromAccelerationStructures));
	defRenderBufferData.def(luabind::constructor<>());
	defRenderBufferData.def_readwrite("stateFlags", &rendering::RenderBufferData::stateFlags);
	defRenderBufferData.def_readwrite("pipelineSpecializationFlags", &rendering::RenderBufferData::pipelineSpecializationFlags);
	defRenderBufferData.def_readwrite("material", &rendering::RenderBufferData::material);
	defRenderBufferData.def_readwrite("renderBuffer", &rendering::RenderBufferData::renderBuffer);
	defCModel.scope[defRenderBufferData];

	auto defPropBlock = luabind::class_<rendering::MaterialPropertyBlock>("MaterialPropertyBlock");
	defPropBlock.def(luabind::constructor<>());
	defPropBlock.def("GetPropertyBlock", &rendering::MaterialPropertyBlock::GetPropertyBlock);
	defPropBlock.def("GetTextureBlock", &rendering::MaterialPropertyBlock::GetTextureBlock);
	defCModel.scope[defPropBlock];

	modEnts[defCModel];

	pragma::LuaCore::define_custom_constructor<rendering::RenderBufferData, +[](const rendering::RenderBufferData &renderBufferData) -> rendering::RenderBufferData { return renderBufferData; }, const rendering::RenderBufferData &>(l);
}
