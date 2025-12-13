// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cstddef>

module pragma.client;

import :entities.components.raytracing;
import :engine;
import :game;
import :model;
import :rendering.shaders;

using namespace pragma;

static std::shared_ptr<prosper::IUniformResizableBuffer> s_entityMeshInfoBuffer = nullptr;
static uint32_t m_entityMeshCount = 0;
static std::shared_ptr<material::MaterialDescriptorArrayManager> s_materialDescriptorArrayManager = nullptr;
static std::shared_ptr<prosper::IDescriptorSetGroup> s_gameSceneDsg = nullptr;
static bool s_allResourcesInitialized = false;
void CRaytracingComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) {}
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
	s_entityMeshInfoBuffer = get_cengine()->GetRenderContext().CreateUniformResizableBuffer(createInfo, instanceSize, instanceSize * maxInstanceCount, 0.1f);
	s_entityMeshInfoBuffer->SetDebugName("entity_mesh_info_buf");

	s_gameSceneDsg = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderRayTracing::DESCRIPTOR_SET_GAME_SCENE);
	s_materialDescriptorArrayManager = prosper::DescriptorArrayManager::Create<material::MaterialDescriptorArrayManager>(s_gameSceneDsg, math::to_integral(ShaderRayTracing::GameSceneBinding::TextureArray));

	auto &ds = *s_gameSceneDsg->GetDescriptorSet();
	ds.SetBindingStorageBuffer(*s_materialDescriptorArrayManager->GetMaterialInfoBuffer(), math::to_integral(ShaderRayTracing::GameSceneBinding::MaterialInfos));
	ds.SetBindingStorageBuffer(*s_entityMeshInfoBuffer, math::to_integral(ShaderRayTracing::GameSceneBinding::SubMeshInfos));
	ds.SetBindingStorageBuffer(*CRenderComponent::GetInstanceBuffer(), math::to_integral(ShaderRayTracing::GameSceneBinding::EntityInstanceData));
	ds.SetBindingStorageBuffer(*get_instance_bone_buffer(), math::to_integral(ShaderRayTracing::GameSceneBinding::BoneMatrices));
	ds.SetBindingStorageBuffer(*geometry::CModelSubMesh::GetGlobalVertexBuffer(), math::to_integral(ShaderRayTracing::GameSceneBinding::VertexBuffer));
	ds.SetBindingStorageBuffer(*geometry::CModelSubMesh::GetGlobalIndexBuffer(), math::to_integral(ShaderRayTracing::GameSceneBinding::IndexBuffer));
	ds.SetBindingStorageBuffer(*geometry::CModelSubMesh::GetGlobalVertexWeightBuffer(), math::to_integral(ShaderRayTracing::GameSceneBinding::VertexWeightBuffer));
	ds.SetBindingStorageBuffer(*geometry::CModelSubMesh::GetGlobalAlphaBuffer(), math::to_integral(ShaderRayTracing::GameSceneBinding::AlphaBuffer));

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
static auto cvRenderTechnique = console::get_client_con_var("render_technique");
bool CRaytracingComponent::IsRaytracingEnabled() { return cvRenderTechnique->GetBool() && s_allResourcesInitialized; }
const std::shared_ptr<prosper::IUniformResizableBuffer> &CRaytracingComponent::GetEntityMeshInfoBuffer() { return s_entityMeshInfoBuffer; }
uint32_t CRaytracingComponent::GetBufferMeshCount() { return m_entityMeshCount; }
const std::shared_ptr<material::MaterialDescriptorArrayManager> &CRaytracingComponent::GetMaterialDescriptorArrayManager() { return s_materialDescriptorArrayManager; }
const std::shared_ptr<prosper::IDescriptorSetGroup> &CRaytracingComponent::GetGameSceneDescriptorSetGroup() { return s_gameSceneDsg; }

CRaytracingComponent::CRaytracingComponent(ecs::BaseEntity &ent) : BaseEntityComponent {ent} {}
CRaytracingComponent::~CRaytracingComponent() { m_subMeshBuffers.clear(); }
void CRaytracingComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CRaytracingComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	// TODO
	//BindEventUnhandled(CAnimatedComponent::EVENT_ON_BONE_BUFFER_INITIALIZED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
	//	SetBoneBufferDirty();
	//});
	BindEventUnhandled(cRenderComponent::EVENT_ON_RENDER_BUFFERS_INITIALIZED, [this](std::reference_wrapper<ComponentEvent> evData) { SetRenderBufferDirty(); });
	BindEventUnhandled(cModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) { InitializeModelRaytracingBuffers(); });
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
	auto renderMode = renderC.valid() ? renderC->GetSceneRenderPass() : rendering::SceneRenderPass::World;
	auto flags = SubMeshRenderInfoBufferData::Flags::None;
	switch(renderMode) {
	case rendering::SceneRenderPass::World:
		flags |= SubMeshRenderInfoBufferData::Flags::RenderModeWorld;
		break;
	case rendering::SceneRenderPass::View:
		flags |= SubMeshRenderInfoBufferData::Flags::RenderModeView;
		break;
	case rendering::SceneRenderPass::Sky:
		flags |= SubMeshRenderInfoBufferData::Flags::RenderModeSkybox;
		break;
		// case pragma::rendering::SceneRenderPass::Water:
		// 	flags |= SubMeshRenderInfoBufferData::Flags::RenderModeWater;
		// 	break;
	}

	std::vector<std::shared_ptr<geometry::ModelMesh>> lodMeshes {};
	std::vector<uint32_t> bodyGroups {};
	bodyGroups.resize(mdl->GetBodyGroupCount());
	mdl->GetBodyGroupMeshes(bodyGroups, 0, lodMeshes);
	for(auto &mesh : lodMeshes) {
		for(auto &subMesh : mesh->GetSubMeshes()) {
			auto &cSubMesh = static_cast<geometry::CModelSubMesh &>(*subMesh);
			auto &vkMesh = cSubMesh.GetSceneMesh();
			if(vkMesh == nullptr)
				continue;
			auto matIdx = mdl->GetMaterialIndex(cSubMesh);
			auto *mat = matIdx.has_value() ? mdl->GetMaterial(*matIdx, ent.GetSkin()) : nullptr;
			if(mat == nullptr)
				continue;
			auto matArrayIndex = s_materialDescriptorArrayManager->RegisterMaterial(*mat);

			SubMeshRenderInfoBufferData subMeshBufferData {};

			static_assert((sizeof(geometry::CModelSubMesh::VertexType) % sizeof(Vector4)) == 0, "Invalid base alignment for Vertex structure!");
			auto &vertexBuffer = vkMesh->GetVertexBuffer();
			if(vertexBuffer)
				subMeshBufferData.vertexBufferStartIndex = vertexBuffer->GetStartOffset() / sizeof(geometry::CModelSubMesh::VertexType);

			auto &indexBuffer = vkMesh->GetIndexBuffer();
			if(indexBuffer)
				subMeshBufferData.indexBufferStartIndex = indexBuffer->GetStartOffset() / sizeof(geometry::IndexType);

			auto &vertexWeightBuffer = vkMesh->GetVertexWeightBuffer();
			if(vertexWeightBuffer)
				subMeshBufferData.vertexWeightBufferIndex = vertexWeightBuffer->GetStartOffset() / sizeof(geometry::CModelSubMesh::VertexWeightType);

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
	if(math::is_flag_set(m_stateFlags, StateFlags::RenderBufferDirty)) {
		math::set_flag(m_stateFlags, StateFlags::RenderBufferDirty, false);
		auto &renderComponent = *whRenderComponent;
		auto *renderBuffer = renderComponent.GetRenderBuffer();
		auto index = renderBuffer->GetBaseIndex(); //wpRenderBuffer ? static_cast<prosper::IBuffer::SmallOffset>(wpRenderBuffer->GetBaseIndex()) : prosper::IBuffer::INVALID_SMALL_OFFSET;
		for(auto &buf : m_subMeshBuffers)
			cmd.RecordUpdateGenericShaderReadBuffer(*buf, offsetof(SubMeshRenderInfoBufferData, entityBufferIndex), sizeof(index), &index);
	}
	if(math::is_flag_set(m_stateFlags, StateFlags::BoneBufferDirty)) {
		auto whAnimatedComponent = GetEntity().GetComponent<CAnimatedComponent>();
		math::set_flag(m_stateFlags, StateFlags::BoneBufferDirty, false);
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
	math::set_flag(m_stateFlags, StateFlags::RenderBufferDirty);
	InitializeBufferUpdateCallback();
}
void CRaytracingComponent::SetBoneBufferDirty()
{
	math::set_flag(m_stateFlags, StateFlags::BoneBufferDirty);
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

static void cmd_render_technique(NetworkState *, const console::ConVar &, int32_t, int32_t val)
{
	if(get_cgame() == nullptr)
		return;
	enum class RenderingTechnique : uint8_t { Rasterization, Raytracing };

	ecs::EntityIterator entIt {*get_cgame()};
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
namespace {
	auto _ = pragma::console::client::register_variable_listener<int32_t>("render_technique", &cmd_render_technique);
}
