#include "stdafx_client.h"
#include "pragma/entities/components/c_raytracing_component.hpp"
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
#include <material_descriptor_array.hpp>

using namespace pragma;

extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;

#pragma optimize("",off)
static std::shared_ptr<prosper::UniformResizableBuffer> s_entityMeshInfoBuffer = nullptr;
static uint32_t m_entityMeshCount = 0;
static std::shared_ptr<MaterialDescriptorArrayManager> s_materialDescriptorArrayManager = nullptr;
static std::shared_ptr<prosper::DescriptorSetGroup> s_gameSceneDsg = nullptr;
static bool s_allResourcesInitialized = false;
void CRaytracingComponent::RegisterEvents(pragma::EntityComponentManager &componentManager) {}
bool CRaytracingComponent::InitializeBuffers()
{
	if(s_allResourcesInitialized)
		return false;
	auto instanceSize = sizeof(SubMeshRenderInfoBufferData);
	auto instanceCount = 32'768;
	auto maxInstanceCount = instanceCount *10u;
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	createInfo.size = instanceSize *instanceCount;
	createInfo.usageFlags = Anvil::BufferUsageFlagBits::STORAGE_BUFFER_BIT | Anvil::BufferUsageFlagBits::TRANSFER_SRC_BIT | Anvil::BufferUsageFlagBits::TRANSFER_DST_BIT;
	m_entityMeshCount = 0;
	s_entityMeshInfoBuffer = prosper::util::create_uniform_resizable_buffer(*c_engine,createInfo,instanceSize,instanceSize *maxInstanceCount,0.1f);
	s_entityMeshInfoBuffer->SetDebugName("entity_mesh_info_buf");
	
	s_gameSceneDsg = prosper::util::create_descriptor_set_group(c_engine->GetDevice(),pragma::ShaderRayTracing::DESCRIPTOR_SET_GAME_SCENE);
	s_materialDescriptorArrayManager = prosper::DescriptorArrayManager::Create<MaterialDescriptorArrayManager>(s_gameSceneDsg,umath::to_integral(pragma::ShaderRayTracing::GameSceneBinding::TextureArray));

	auto &ds = *(*s_gameSceneDsg)->get_descriptor_set(0);
	prosper::util::set_descriptor_set_binding_storage_buffer(ds,*s_materialDescriptorArrayManager->GetMaterialInfoBuffer(),umath::to_integral(pragma::ShaderRayTracing::GameSceneBinding::MaterialInfos));
	prosper::util::set_descriptor_set_binding_storage_buffer(ds,*s_entityMeshInfoBuffer,umath::to_integral(pragma::ShaderRayTracing::GameSceneBinding::SubMeshInfos));
	prosper::util::set_descriptor_set_binding_storage_buffer(ds,*CRenderComponent::GetInstanceBuffer(),umath::to_integral(pragma::ShaderRayTracing::GameSceneBinding::EntityInstanceData));
	prosper::util::set_descriptor_set_binding_storage_buffer(ds,*pragma::get_instance_bone_buffer(),umath::to_integral(pragma::ShaderRayTracing::GameSceneBinding::BoneMatrices));
	prosper::util::set_descriptor_set_binding_storage_buffer(ds,*CModelSubMesh::GetGlobalVertexBuffer(),umath::to_integral(pragma::ShaderRayTracing::GameSceneBinding::VertexBuffer));
	prosper::util::set_descriptor_set_binding_storage_buffer(ds,*CModelSubMesh::GetGlobalIndexBuffer(),umath::to_integral(pragma::ShaderRayTracing::GameSceneBinding::IndexBuffer));
	prosper::util::set_descriptor_set_binding_storage_buffer(ds,*CModelSubMesh::GetGlobalVertexWeightBuffer(),umath::to_integral(pragma::ShaderRayTracing::GameSceneBinding::VertexWeightBuffer));
	prosper::util::set_descriptor_set_binding_storage_buffer(ds,*CModelSubMesh::GetGlobalAlphaBuffer(),umath::to_integral(pragma::ShaderRayTracing::GameSceneBinding::AlphaBuffer));

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
bool CRaytracingComponent::IsRaytracingEnabled()
{
	return cvRenderTechnique->GetBool() && s_allResourcesInitialized;
}
const std::shared_ptr<prosper::UniformResizableBuffer> &CRaytracingComponent::GetEntityMeshInfoBuffer() {return s_entityMeshInfoBuffer;}
uint32_t CRaytracingComponent::GetBufferMeshCount() {return m_entityMeshCount;}
const std::shared_ptr<MaterialDescriptorArrayManager> &CRaytracingComponent::GetMaterialDescriptorArrayManager() {return s_materialDescriptorArrayManager;}
const std::shared_ptr<prosper::DescriptorSetGroup> &CRaytracingComponent::GetGameSceneDescriptorSetGroup() {return s_gameSceneDsg;}

CRaytracingComponent::CRaytracingComponent(BaseEntity &ent)
	: BaseEntityComponent{ent}
{}
CRaytracingComponent::~CRaytracingComponent()
{
	m_subMeshBuffers.clear();
}
luabind::object CRaytracingComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CRaytracingComponentHandleWrapper>(l);}
void CRaytracingComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEventUnhandled(CAnimatedComponent::EVENT_ON_BONE_BUFFER_INITIALIZED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		UpdateBoneBuffer();
	});
	BindEventUnhandled(CRenderComponent::EVENT_ON_RENDER_BUFFERS_INITIALIZED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		UpdateRenderBuffer();
	});
	BindEventUnhandled(CModelComponent::EVENT_ON_MODEL_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		UpdateModelRaytracingBuffers();
	});
	UpdateModelRaytracingBuffers();
}
void CRaytracingComponent::UpdateModelRaytracingBuffers()
{
	if(s_entityMeshInfoBuffer == nullptr || s_materialDescriptorArrayManager == nullptr)
		return;
	auto wpMdlComponent = GetEntity().GetModelComponent();
	auto mdl = wpMdlComponent.valid() ? wpMdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
	{
		m_subMeshBuffers.clear();
		return;
	}

	for(auto &meshGroup : mdl->GetMeshGroups())
	{
		for(auto &mesh : meshGroup->GetMeshes())
		{
			for(auto &subMesh : mesh->GetSubMeshes())
			{
				auto &cSubMesh = static_cast<CModelSubMesh&>(*subMesh);
				auto &vkMesh = cSubMesh.GetVKMesh();
				if(vkMesh == nullptr)
					continue;
				auto matIdx = cSubMesh.GetTexture();
				auto *mat = mdl->GetMaterial(matIdx);
				if(mat == nullptr)
					continue;
				auto matArrayIndex = s_materialDescriptorArrayManager->RegisterMaterial(*mat);

				SubMeshRenderInfoBufferData subMeshBufferData {};

				static_assert((sizeof(CModelSubMesh::VertexType) %sizeof(Vector4)) == 0,"Invalid base alignment for Vertex structure!");
				auto &vertexBuffer = vkMesh->GetVertexBuffer();
				if(vertexBuffer)
					subMeshBufferData.vertexBufferStartIndex = vertexBuffer->GetStartOffset() /sizeof(CModelSubMesh::VertexType);

				auto &indexBuffer = vkMesh->GetIndexBuffer();
				if(indexBuffer)
					subMeshBufferData.indexBufferStartIndex = indexBuffer->GetStartOffset() /sizeof(CModelSubMesh::IndexType);

				auto &vertexWeightBuffer = vkMesh->GetVertexWeightBuffer();
				if(vertexWeightBuffer)
					subMeshBufferData.vertexWeightBufferIndex = vertexWeightBuffer->GetStartOffset() /sizeof(CModelSubMesh::VertexWeightType);

				if(matArrayIndex.has_value())
					subMeshBufferData.materialArrayIndex = *matArrayIndex;

				subMeshBufferData.flags = SubMeshRenderInfoBufferData::Flags::Visible;
				Vector3 min,max;
				subMesh->GetBounds(min,max);
				subMeshBufferData.aabbMin = {min.x,min.y,min.z,0.f};
				subMeshBufferData.aabbMax = {max.x,max.y,max.z,0.f};
				subMeshBufferData.numTriangles = subMesh->GetTriangleCount();
				auto buf = s_entityMeshInfoBuffer->AllocateBuffer(&subMeshBufferData);
				if(buf)
				{
					m_subMeshBuffers.push_back(buf);
					m_entityMeshCount = std::max(m_entityMeshCount,buf->GetBaseIndex() +1);
				}
			}
		}
	}
	UpdateRenderBuffer();
	UpdateBoneBuffer();
}
void CRaytracingComponent::UpdateRenderBuffer()
{
	auto whRenderComponent = GetEntity().GetComponent<CRenderComponent>();
	if(whRenderComponent.expired())
		return;
	auto &renderComponent = *whRenderComponent;
	auto wpRenderBuffer = renderComponent.GetRenderBuffer();
	auto index = wpRenderBuffer.expired() == false ? static_cast<prosper::Buffer::SmallOffset>(wpRenderBuffer.lock()->GetBaseIndex()) : prosper::Buffer::INVALID_SMALL_OFFSET;
	for(auto &buf : m_subMeshBuffers)
		c_engine->ScheduleRecordUpdateBuffer(buf,offsetof(SubMeshRenderInfoBufferData,entityBufferIndex),sizeof(index),&index);
}
void CRaytracingComponent::UpdateBoneBuffer()
{
	auto whAnimatedComponent = GetEntity().GetComponent<CAnimatedComponent>();
	if(whAnimatedComponent.expired())
		return;
	auto &animatedComponent = *whAnimatedComponent;
	auto wpAnimatedComponent = animatedComponent.GetBoneBuffer();
	auto index = wpAnimatedComponent.expired() == false ? static_cast<prosper::Buffer::SmallOffset>(wpAnimatedComponent.lock()->GetBaseIndex()) : prosper::Buffer::INVALID_SMALL_OFFSET;
	for(auto &buf : m_subMeshBuffers)
		c_engine->ScheduleRecordUpdateBuffer(buf,offsetof(SubMeshRenderInfoBufferData,boneBufferStartIndex),sizeof(index),&index);
}

static void cmd_render_technique(NetworkState*,ConVar*,int32_t,int32_t val)
{
	if(c_game == nullptr)
		return;
	enum class RenderingTechnique : uint8_t
	{
		Rasterization,
		Raytracing
	};

	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CRenderComponent>>();
	auto technique = static_cast<RenderingTechnique>(val);
	switch(technique)
	{
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
REGISTER_CONVAR_CALLBACK_CL(render_technique,cmd_render_technique);

#if 0
void CRaytracingComponent::InitializeMeshForRayTracing()
{
				// Array of textures

				// Array of materials

				struct SubMeshInfo
				{

					// ??
				};
				// Array of sub-meshes

	// Material:
	// Diffuse Map -> Index 0
	// Specular Map -> Index 1
	// etc.
	// But: Material used for multiple descriptor arrays?
	// KEIN Global array
	// Wie Indices in texturen speichern?

	// Raytracing:
	// Vector<SubMesh>
	// For each submesh: Which vertex buffer? Which Index Buffer? Which Material parameters? Which diffuse map, specular map, etc.?
	// For each submesh: Index into global scene arr\ay
	// Add to render component? -> When removed, remove from global scene array

			TODO:
				When to deregister material from MaterialDescriptorArrayManager?
}
#endif
#pragma optimize("",on)
