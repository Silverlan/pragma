/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/c_engine.h"
#include "pragma/entities/components/c_raytracing_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include "pragma/rendering/shaders/world/raytracing/c_shader_raytracing.hpp"
#include "pragma/console/c_cvar.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <buffers/prosper_dynamic_resizable_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_command_buffer.hpp>
#include <material_descriptor_array.hpp>
#include <buffers/prosper_uniform_resizable_buffer.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

using namespace pragma;

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT CEngine *c_engine;

static std::shared_ptr<prosper::IUniformResizableBuffer> s_entityMeshInfoBuffer = nullptr;
static uint32_t m_entityMeshCount = 0;
static std::shared_ptr<MaterialDescriptorArrayManager> s_materialDescriptorArrayManager = nullptr;
static std::shared_ptr<prosper::IDescriptorSetGroup> s_gameSceneDsg = nullptr;
static bool s_allResourcesInitialized = false;
void CRaytracingComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) {}
bool CRaytracingComponent::InitializeBuffers()
{
	if(s_allResourcesInitialized)
		return true;
	auto instanceSize = sizeof(SubMeshRenderInfoBufferData);
	auto instanceCount = 32'768;
	auto maxInstanceCount = instanceCount * 10u;
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	createInfo.size = instanceSize * instanceCount;
	createInfo.usageFlags = prosper::BufferUsageFlags::StorageBufferBit | prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::TransferDstBit;
	m_entityMeshCount = 0;
	s_entityMeshInfoBuffer = c_engine->GetRenderContext().CreateUniformResizableBuffer(createInfo, instanceSize, instanceSize * maxInstanceCount, 0.1f);
	s_entityMeshInfoBuffer->SetDebugName("entity_mesh_info_buf");

	s_gameSceneDsg = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderRayTracing::DESCRIPTOR_SET_GAME_SCENE);
	s_materialDescriptorArrayManager = prosper::DescriptorArrayManager::Create<MaterialDescriptorArrayManager>(s_gameSceneDsg, umath::to_integral(pragma::ShaderRayTracing::GameSceneBinding::TextureArray));

	auto &ds = *s_gameSceneDsg->GetDescriptorSet();
	ds.SetBindingStorageBuffer(*s_materialDescriptorArrayManager->GetMaterialInfoBuffer(), umath::to_integral(pragma::ShaderRayTracing::GameSceneBinding::MaterialInfos));
	ds.SetBindingStorageBuffer(*s_entityMeshInfoBuffer, umath::to_integral(pragma::ShaderRayTracing::GameSceneBinding::SubMeshInfos));
	ds.SetBindingStorageBuffer(*CRenderComponent::GetInstanceBuffer(), umath::to_integral(pragma::ShaderRayTracing::GameSceneBinding::EntityInstanceData));
	ds.SetBindingStorageBuffer(*pragma::get_instance_bone_buffer(), umath::to_integral(pragma::ShaderRayTracing::GameSceneBinding::BoneMatrices));
	ds.SetBindingStorageBuffer(*CModelSubMesh::GetGlobalVertexBuffer(), umath::to_integral(pragma::ShaderRayTracing::GameSceneBinding::VertexBuffer));
	ds.SetBindingStorageBuffer(*CModelSubMesh::GetGlobalIndexBuffer(), umath::to_integral(pragma::ShaderRayTracing::GameSceneBinding::IndexBuffer));
	ds.SetBindingStorageBuffer(*CModelSubMesh::GetGlobalVertexWeightBuffer(), umath::to_integral(pragma::ShaderRayTracing::GameSceneBinding::VertexWeightBuffer));
	ds.SetBindingStorageBuffer(*CModelSubMesh::GetGlobalAlphaBuffer(), umath::to_integral(pragma::ShaderRayTracing::GameSceneBinding::AlphaBuffer));

	s_allResourcesInitialized = s_entityMeshInfoBuffer && s_materialDescriptorArrayManager && s_gameSceneDsg;
	return s_allResourcesInitialized;
}
void CRaytracingComponent::ClearBuffers()
{
	m_entityMeshCount = 0;
	s_entityMeshInfoBuffer = nullptr;
	s_materialDescriptorArrayManager = nullptr;
	s_gameSceneDsg = nullptr;
	s_allResourcesInitialized = false;
}
static auto cvRenderTechnique = GetClientConVar("render_technique");
bool CRaytracingComponent::IsRaytracingEnabled() { return cvRenderTechnique->GetBool() && s_allResourcesInitialized; }
const std::shared_ptr<prosper::IUniformResizableBuffer> &CRaytracingComponent::GetEntityMeshInfoBuffer() { return s_entityMeshInfoBuffer; }
uint32_t CRaytracingComponent::GetBufferMeshCount() { return m_entityMeshCount; }
const std::shared_ptr<MaterialDescriptorArrayManager> &CRaytracingComponent::GetMaterialDescriptorArrayManager() { return s_materialDescriptorArrayManager; }
const std::shared_ptr<prosper::IDescriptorSetGroup> &CRaytracingComponent::GetGameSceneDescriptorSetGroup() { return s_gameSceneDsg; }

CRaytracingComponent::CRaytracingComponent(BaseEntity &ent) : BaseEntityComponent {ent} {}
CRaytracingComponent::~CRaytracingComponent() { m_subMeshBuffers.clear(); }
void CRaytracingComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CRaytracingComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	// TODO
	//BindEventUnhandled(CAnimatedComponent::EVENT_ON_BONE_BUFFER_INITIALIZED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
	//	SetBoneBufferDirty();
	//});
	BindEventUnhandled(CRenderComponent::EVENT_ON_RENDER_BUFFERS_INITIALIZED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { SetRenderBufferDirty(); });
	BindEventUnhandled(CModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { InitializeModelRaytracingBuffers(); });
	InitializeModelRaytracingBuffers();
}
void CRaytracingComponent::InitializeModelRaytracingBuffers()
{
	if(s_entityMeshInfoBuffer == nullptr || s_materialDescriptorArrayManager == nullptr)
		return;
	auto &ent = GetEntity();
	auto &mdl = ent.GetModel();
	if(mdl == nullptr) {
		m_subMeshBuffers.clear();
		return;
	}
	auto renderC = ent.GetComponent<CRenderComponent>();
	auto renderMode = renderC.valid() ? renderC->GetSceneRenderPass() : pragma::rendering::SceneRenderPass::World;
	auto flags = SubMeshRenderInfoBufferData::Flags::None;
	switch(renderMode) {
	case pragma::rendering::SceneRenderPass::World:
		flags |= SubMeshRenderInfoBufferData::Flags::RenderModeWorld;
		break;
	case pragma::rendering::SceneRenderPass::View:
		flags |= SubMeshRenderInfoBufferData::Flags::RenderModeView;
		break;
	case pragma::rendering::SceneRenderPass::Sky:
		flags |= SubMeshRenderInfoBufferData::Flags::RenderModeSkybox;
		break;
		// case pragma::rendering::SceneRenderPass::Water:
		// 	flags |= SubMeshRenderInfoBufferData::Flags::RenderModeWater;
		// 	break;
	}

	std::vector<std::shared_ptr<ModelMesh>> lodMeshes {};
	std::vector<uint32_t> bodyGroups {};
	bodyGroups.resize(mdl->GetBodyGroupCount());
	mdl->GetBodyGroupMeshes(bodyGroups, 0, lodMeshes);
	for(auto &mesh : lodMeshes) {
		for(auto &subMesh : mesh->GetSubMeshes()) {
			auto &cSubMesh = static_cast<CModelSubMesh &>(*subMesh);
			auto &vkMesh = cSubMesh.GetSceneMesh();
			if(vkMesh == nullptr)
				continue;
			auto matIdx = mdl->GetMaterialIndex(cSubMesh);
			auto *mat = matIdx.has_value() ? mdl->GetMaterial(*matIdx, ent.GetSkin()) : nullptr;
			if(mat == nullptr)
				continue;
			auto matArrayIndex = s_materialDescriptorArrayManager->RegisterMaterial(*mat);

			SubMeshRenderInfoBufferData subMeshBufferData {};

			static_assert((sizeof(CModelSubMesh::VertexType) % sizeof(Vector4)) == 0, "Invalid base alignment for Vertex structure!");
			auto &vertexBuffer = vkMesh->GetVertexBuffer();
			if(vertexBuffer)
				subMeshBufferData.vertexBufferStartIndex = vertexBuffer->GetStartOffset() / sizeof(CModelSubMesh::VertexType);

			auto &indexBuffer = vkMesh->GetIndexBuffer();
			if(indexBuffer)
				subMeshBufferData.indexBufferStartIndex = indexBuffer->GetStartOffset() / sizeof(pragma::model::IndexType);

			auto &vertexWeightBuffer = vkMesh->GetVertexWeightBuffer();
			if(vertexWeightBuffer)
				subMeshBufferData.vertexWeightBufferIndex = vertexWeightBuffer->GetStartOffset() / sizeof(CModelSubMesh::VertexWeightType);

			if(matArrayIndex.has_value())
				subMeshBufferData.materialArrayIndex = *matArrayIndex;

			subMeshBufferData.flags = flags | SubMeshRenderInfoBufferData::Flags::Visible;
			if(mat->GetNormalMap())
				subMeshBufferData.flags |= SubMeshRenderInfoBufferData::Flags::UseNormalMap;

			Vector3 min, max;
			subMesh->GetBounds(min, max);
			subMeshBufferData.aabbMin = {min.x, min.y, min.z, 0.f};
			subMeshBufferData.aabbMax = {max.x, max.y, max.z, 0.f};
			subMeshBufferData.numTriangles = subMesh->GetTriangleCount();
			auto buf = s_entityMeshInfoBuffer->AllocateBuffer(&subMeshBufferData);
			if(buf) {
				m_subMeshBuffers.push_back(buf);
				m_entityMeshCount = std::max(m_entityMeshCount, buf->GetBaseIndex() + 1);
			}
		}
	}
	SetRenderBufferDirty();
	SetBoneBufferDirty();
}
void CRaytracingComponent::UpdateBuffers(prosper::IPrimaryCommandBuffer &cmd)
{
	auto whRenderComponent = GetEntity().GetComponent<CRenderComponent>();
	if(whRenderComponent.expired())
		return;
	if(umath::is_flag_set(m_stateFlags, StateFlags::RenderBufferDirty)) {
		umath::set_flag(m_stateFlags, StateFlags::RenderBufferDirty, false);
		auto &renderComponent = *whRenderComponent;
		auto *renderBuffer = renderComponent.GetRenderBuffer();
		auto index = renderBuffer->GetBaseIndex(); //wpRenderBuffer ? static_cast<prosper::IBuffer::SmallOffset>(wpRenderBuffer->GetBaseIndex()) : prosper::IBuffer::INVALID_SMALL_OFFSET;
		for(auto &buf : m_subMeshBuffers)
			cmd.RecordUpdateGenericShaderReadBuffer(*buf, offsetof(SubMeshRenderInfoBufferData, entityBufferIndex), sizeof(index), &index);
	}
	if(umath::is_flag_set(m_stateFlags, StateFlags::BoneBufferDirty)) {
		auto whAnimatedComponent = GetEntity().GetComponent<CAnimatedComponent>();
		umath::set_flag(m_stateFlags, StateFlags::BoneBufferDirty, false);
		auto wpBoneBuffer = whAnimatedComponent->GetBoneBuffer(); //whAnimatedComponent.valid() ? whAnimatedComponent->GetBoneBuffer() : std::weak_ptr<prosper::IBuffer>{};
		auto index = wpBoneBuffer ? static_cast<prosper::IBuffer::SmallOffset>(wpBoneBuffer->GetBaseIndex()) : prosper::IBuffer::INVALID_SMALL_OFFSET;
		for(auto &buf : m_subMeshBuffers)
			cmd.RecordUpdateGenericShaderReadBuffer(*buf, offsetof(SubMeshRenderInfoBufferData, boneBufferStartIndex), sizeof(index), &index);
	}
	if(m_cbUpdateBuffers.IsValid())
		m_cbUpdateBuffers.Remove();
}
void CRaytracingComponent::SetRenderBufferDirty()
{
	umath::set_flag(m_stateFlags, StateFlags::RenderBufferDirty);
	InitializeBufferUpdateCallback();
}
void CRaytracingComponent::SetBoneBufferDirty()
{
	umath::set_flag(m_stateFlags, StateFlags::BoneBufferDirty);
	InitializeBufferUpdateCallback();
}
void CRaytracingComponent::InitializeBufferUpdateCallback()
{
	if(m_cbUpdateBuffers.IsValid())
		return;
	auto renderC = GetEntity().GetComponent<CRenderComponent>();
	if(renderC.valid())
		renderC->SetRenderBufferDirty();
	// TODO
	//m_cbUpdateBuffers = BindEventUnhandled(CRenderComponent::EVENT_ON_UPDATE_RENDER_DATA,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
	//	UpdateBuffers(*static_cast<CEOnUpdateRenderData&>(evData.get()).commandBuffer);
	//});
}

static void cmd_render_technique(NetworkState *, const ConVar &, int32_t, int32_t val)
{
	if(c_game == nullptr)
		return;
	enum class RenderingTechnique : uint8_t { Rasterization, Raytracing };

	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CRenderComponent>>();
	auto technique = static_cast<RenderingTechnique>(val);
	switch(technique) {
	case RenderingTechnique::Raytracing:
		{
			CRaytracingComponent::InitializeBuffers();
			for(auto *ent : entIt)
				ent->AddComponent<CRaytracingComponent>();
			break;
		}
	default:
		{
			CRaytracingComponent::ClearBuffers();
			for(auto *ent : entIt)
				ent->RemoveComponent<CRaytracingComponent>();
			break;
		}
	}
}
REGISTER_CONVAR_CALLBACK_CL(render_technique, cmd_render_technique);
