/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/lighting/c_light_data_buffer_manager.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/entities/entity_instance_index_buffer.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

decltype(ShaderScene::DESCRIPTOR_SET_RENDER_SETTINGS) ShaderScene::DESCRIPTOR_SET_RENDER_SETTINGS = {
	{
		prosper::DescriptorSetInfo::Binding { // Debug
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Time
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit
		},
		prosper::DescriptorSetInfo::Binding { // CSM Data
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Global Entity Instance Data
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit
		}
	}
};
decltype(ShaderScene::DESCRIPTOR_SET_SCENE) ShaderScene::DESCRIPTOR_SET_SCENE = {
	{
		prosper::DescriptorSetInfo::Binding { // Camera
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::GeometryBit
		},
		prosper::DescriptorSetInfo::Binding { // Render Settings
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::GeometryBit
		}
	}
};
decltype(ShaderScene::DESCRIPTOR_SET_RENDERER) ShaderScene::DESCRIPTOR_SET_RENDERER = {
	{
		prosper::DescriptorSetInfo::Binding { // Renderer
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // SSAO Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Light Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
decltype(ShaderScene::RENDER_PASS_FORMAT) ShaderScene::RENDER_PASS_FORMAT = prosper::Format::R16G16B16A16_SFloat;
decltype(ShaderScene::RENDER_PASS_DEPTH_FORMAT) ShaderScene::RENDER_PASS_DEPTH_FORMAT = prosper::Format::D32_SFloat;
decltype(ShaderScene::RENDER_PASS_SAMPLES) ShaderScene::RENDER_PASS_SAMPLES = prosper::SampleCountFlags::e1Bit;
void ShaderScene::SetRenderPassSampleCount(prosper::SampleCountFlags samples) {RENDER_PASS_SAMPLES = samples;}
ShaderScene::ShaderScene(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: Shader3DBase(context,identifier,vsShader,fsShader,gsShader)
{
	//SetPipelineCount(umath::to_integral(Pipeline::Count));
}
bool ShaderScene::ShouldInitializePipeline(uint32_t pipelineIdx)
{
	//if(static_cast<Pipeline>(pipelineIdx) != Pipeline::MultiSample)
	//	return true;
	//return RENDER_PASS_SAMPLES != prosper::SampleCountFlags::e1Bit;
	return true;
}
prosper::SampleCountFlags ShaderScene::GetSampleCount(uint32_t pipelineIdx) const
{
	return prosper::SampleCountFlags::e1Bit;//(static_cast<Pipeline>(pipelineIdx) == Pipeline::MultiSample) ? RENDER_PASS_SAMPLES : prosper::SampleCountFlags::e1Bit;
}
void ShaderScene::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	auto sampleCount = GetSampleCount(pipelineIdx);
	prosper::util::RenderPassCreateInfo rpCreateInfo {{
		{
			RENDER_PASS_FORMAT,prosper::ImageLayout::ColorAttachmentOptimal,prosper::AttachmentLoadOp::DontCare,
			prosper::AttachmentStoreOp::Store,sampleCount,prosper::ImageLayout::ColorAttachmentOptimal
		},
		{ // Bloom Attachment
			RENDER_PASS_FORMAT,prosper::ImageLayout::ColorAttachmentOptimal,prosper::AttachmentLoadOp::Clear,
			prosper::AttachmentStoreOp::Store,sampleCount,prosper::ImageLayout::ColorAttachmentOptimal
		},
		{
			RENDER_PASS_DEPTH_FORMAT,prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::AttachmentLoadOp::Load,
			prosper::AttachmentStoreOp::Store /* depth values have already been written by prepass */,sampleCount,prosper::ImageLayout::DepthStencilAttachmentOptimal
		}
	}};
	rpCreateInfo.subPasses.push_back(prosper::util::RenderPassCreateInfo::SubPass{std::vector<std::size_t>{0ull,1ull},true});
	/*auto mainSubPassId = rpCreateInfo.subPasses.size() -1ull;

	// Particle sub-pass
	rpCreateInfo.subPasses.push_back(prosper::util::RenderPassCreateInfo::SubPass{std::vector<std::size_t>{0ull,1ull},true});
	auto particleSubPassId = rpCreateInfo.subPasses.size() -1ull;
	auto &subPass = rpCreateInfo.subPasses.back();
	subPass.dependencies.push_back({
		particleSubPassId,mainSubPassId,
		vk::PipelineStageFlagBits::eFragmentShader,vk::PipelineStageFlagBits::eFragmentShader,
		prosper::AccessFlags::ShaderWriteBit | prosper::AccessFlags::ShaderReadBit,prosper::AccessFlags::ShaderWriteBit
	});*/
	CreateCachedRenderPass<ShaderScene>(rpCreateInfo,outRenderPass,pipelineIdx);
}
bool ShaderScene::BindRenderSettings(prosper::IDescriptorSet &descSetRenderSettings)
{
	return RecordBindDescriptorSet(descSetRenderSettings,GetRenderSettingsDescriptorSetIndex());
}
bool ShaderScene::BindSceneCamera(pragma::CSceneComponent &scene,const CRasterizationRendererComponent &renderer,bool bView)
{
	auto *descSet = (bView == true) ? scene.GetViewCameraDescriptorSet() : scene.GetCameraDescriptorSetGraphics();
	return RecordBindDescriptorSet(*descSet,GetCameraDescriptorSetIndex());
}

/////////////////////

decltype(ShaderSceneLit::DESCRIPTOR_SET_LIGHTS) ShaderSceneLit::DESCRIPTOR_SET_LIGHTS = {
	{
		prosper::DescriptorSetInfo::Binding { // Light sources
			LIGHT_SOURCE_BUFFER_TYPE,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Visible light index buffer
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit
		},
		prosper::DescriptorSetInfo::Binding { // Shadow buffers
			LIGHT_SOURCE_BUFFER_TYPE,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Cascade Maps
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit,umath::to_integral(GameLimits::MaxCSMCascades)
		}
	}
};
decltype(ShaderSceneLit::DESCRIPTOR_SET_SHADOWS) ShaderSceneLit::DESCRIPTOR_SET_SHADOWS = {
	{
		prosper::DescriptorSetInfo::Binding { // Shadow Maps
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit,umath::to_integral(GameLimits::MaxActiveShadowMaps)
		},
		prosper::DescriptorSetInfo::Binding { // Shadow Cube-Maps
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit,umath::to_integral(GameLimits::MaxActiveShadowCubeMaps)
		}
	}
};
ShaderSceneLit::ShaderSceneLit(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: ShaderScene(context,identifier,vsShader,fsShader,gsShader)
{}
bool ShaderSceneLit::BindLights(prosper::IDescriptorSet &dsLights)
{
	auto *descSetShadow = pragma::CShadowComponent::GetDescriptorSet();
	if(descSetShadow == nullptr)
		return false;
	return RecordBindDescriptorSets({&dsLights,descSetShadow},GetLightDescriptorSetIndex());
}
bool ShaderSceneLit::BindScene(pragma::CSceneComponent &scene,CRasterizationRendererComponent &renderer,bool bView)
{
	return BindSceneCamera(scene,renderer,bView) && RecordBindDescriptorSet(*renderer.GetRendererDescriptorSet(),GetRendererDescriptorSetIndex()) &&
		BindLights(*renderer.GetLightSourceDescriptorSet());
}

/////////////////////

decltype(ShaderGameWorld::HASH_TYPE) ShaderGameWorld::HASH_TYPE = typeid(ShaderGameWorld).hash_code();
size_t ShaderGameWorld::GetBaseTypeHashCode() const {return HASH_TYPE;}
prosper::IDescriptorSet &ShaderGameWorld::GetDefaultMaterialDescriptorSet() const {return *m_defaultMatDsg->GetDescriptorSet();}

/////////////////////

decltype(ShaderEntity::VERTEX_BINDING_RENDER_BUFFER_INDEX) ShaderEntity::VERTEX_BINDING_RENDER_BUFFER_INDEX = {prosper::VertexInputRate::Instance};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX) ShaderEntity::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX = {VERTEX_BINDING_LIGHTMAP,prosper::Format::R32_UInt};

decltype(ShaderEntity::VERTEX_BINDING_BONE_WEIGHT) ShaderEntity::VERTEX_BINDING_BONE_WEIGHT = {prosper::VertexInputRate::Vertex};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID) ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID = {VERTEX_BINDING_BONE_WEIGHT,prosper::Format::R32G32B32A32_SInt};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT) ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT = {VERTEX_BINDING_BONE_WEIGHT,prosper::Format::R32G32B32A32_SFloat};

decltype(ShaderEntity::VERTEX_BINDING_BONE_WEIGHT_EXT) ShaderEntity::VERTEX_BINDING_BONE_WEIGHT_EXT = {prosper::VertexInputRate::Vertex};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID) ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID = {VERTEX_BINDING_BONE_WEIGHT_EXT,prosper::Format::R32G32B32A32_SInt};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT) ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT = {VERTEX_BINDING_BONE_WEIGHT_EXT,prosper::Format::R32G32B32A32_SFloat};

decltype(ShaderEntity::VERTEX_BINDING_VERTEX) ShaderEntity::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex,sizeof(VertexBufferData)};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_POSITION) ShaderEntity::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_VERTEX,prosper::Format::R32G32B32_SFloat};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_UV) ShaderEntity::VERTEX_ATTRIBUTE_UV = {VERTEX_BINDING_VERTEX,prosper::Format::R32G32_SFloat,offsetof(VertexBufferData,uv)};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_NORMAL) ShaderEntity::VERTEX_ATTRIBUTE_NORMAL = {VERTEX_BINDING_VERTEX,prosper::Format::R32G32B32_SFloat,offsetof(VertexBufferData,normal)};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_TANGENT) ShaderEntity::VERTEX_ATTRIBUTE_TANGENT = {VERTEX_BINDING_VERTEX,prosper::Format::R32G32B32_SFloat,offsetof(VertexBufferData,tangent)};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_BI_TANGENT) ShaderEntity::VERTEX_ATTRIBUTE_BI_TANGENT = {VERTEX_BINDING_VERTEX,prosper::Format::R32G32B32_SFloat,offsetof(VertexBufferData,biTangent)};

decltype(ShaderEntity::VERTEX_BINDING_LIGHTMAP) ShaderEntity::VERTEX_BINDING_LIGHTMAP = {prosper::VertexInputRate::Vertex};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_LIGHTMAP_UV) ShaderEntity::VERTEX_ATTRIBUTE_LIGHTMAP_UV = {VERTEX_BINDING_LIGHTMAP,prosper::Format::R32G32_SFloat};

decltype(ShaderEntity::DESCRIPTOR_SET_INSTANCE) ShaderEntity::DESCRIPTOR_SET_INSTANCE = {
	{
		prosper::DescriptorSetInfo::Binding { // Instance
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit
		},
		prosper::DescriptorSetInfo::Binding { // Bone Matrices
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::VertexBit
		},
		prosper::DescriptorSetInfo::Binding { // Vertex Animations
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::VertexBit
		},
		prosper::DescriptorSetInfo::Binding { // Vertex Animation Frame Data
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::VertexBit
		}
	}
};
ShaderEntity::ShaderEntity(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: ShaderSceneLit(context,identifier,vsShader,fsShader,gsShader)
{}

void ShaderEntity::OnBindEntity(CBaseEntity &ent,CRenderComponent &renderC) {}

bool ShaderEntity::BindEntity(CBaseEntity &ent)
{
	auto pRenderComponent = ent.GetRenderComponent();
	if(!pRenderComponent)
		return false;
	m_boundEntity = &ent;
	auto *descSet = pRenderComponent->GetRenderDescriptorSet();
	if(descSet == nullptr)
	{
		// Con::cwar<<"WARNING: Attempted to render entity "<<ent.GetClass()<<", but it has an invalid render descriptor set! Skipping..."<<Con::endl;
		return false;
	}
	OnBindEntity(ent,*pRenderComponent);
	//if(pRenderComponent->GetLastRenderFrame() != c_engine->GetRenderContext().GetLastFrameId())
	//	Con::cwar<<"WARNING: Entity buffer data for entity "<<ent.GetClass()<<" ("<<ent.GetIndex()<<") hasn't been updated for this frame, but entity is used in rendering! This may cause rendering issues!"<<Con::endl;
	return BindInstanceDescriptorSet(*descSet);
}

void ShaderEntity::EndDraw()
{
	ShaderSceneLit::EndDraw();
	m_boundEntity = nullptr;
}

bool ShaderEntity::GetRenderBufferTargets(
	CModelSubMesh &mesh,uint32_t pipelineIdx,std::vector<prosper::IBuffer*> &outBuffers,std::vector<prosper::DeviceSize> &outOffsets,
	std::optional<prosper::IndexBufferInfo> &outIndexBufferInfo
) const
{
	auto &sceneMesh = mesh.GetSceneMesh();
	if(sceneMesh == nullptr)
		return false;
	auto &indexBuffer = sceneMesh->GetIndexBuffer();
	if(indexBuffer)
	{
		outIndexBufferInfo = prosper::IndexBufferInfo{};
		outIndexBufferInfo->buffer = indexBuffer;
		outIndexBufferInfo->indexType = prosper::IndexType::UInt16;
		outIndexBufferInfo->offset = 0;
	}

	auto *vweightBuf = sceneMesh->GetVertexWeightBuffer().get();
	auto *vweightExtBuf = sceneMesh->GetVertexWeightBuffer().get();
	auto *vBuf = sceneMesh->GetVertexBuffer().get();

	outBuffers = {vweightBuf,vweightBuf,vBuf};
	auto &vertWeights = mesh.GetVertexWeights();
	outOffsets = {
		0ull,
		vertWeights.size() *sizeof(vertWeights.front()),
		0ull
	};
	return true;
}
std::shared_ptr<prosper::IRenderBuffer> ShaderEntity::CreateRenderBuffer(CModelSubMesh &mesh,uint32_t pipelineIdx) const
{
	std::vector<prosper::IBuffer*> buffers;
	std::vector<prosper::DeviceSize> offsets;
	std::optional<prosper::IndexBufferInfo> indexBufferInfo {};
	if(GetRenderBufferTargets(mesh,pipelineIdx,buffers,offsets,indexBufferInfo) == false)
		return nullptr;
	buffers.insert(buffers.begin(),CSceneComponent::GetEntityInstanceIndexBuffer()->GetBuffer().get()); // Instance buffer
	offsets.insert(offsets.begin(),0);
	auto *dummyBuffer = c_engine->GetRenderContext().GetDummyBuffer().get();
	for(auto it=buffers.begin();it!=buffers.end();++it)
	{
		auto *buf = *it;
		*it = buf ? buf : dummyBuffer;
	}
	return GetContext().CreateRenderBuffer(static_cast<const prosper::GraphicsPipelineCreateInfo&>(*GetPipelineCreateInfo(pipelineIdx)),buffers,offsets,indexBufferInfo);
}

CBaseEntity *ShaderEntity::GetBoundEntity() {return m_boundEntity;}

bool ShaderEntity::BindInstanceDescriptorSet(prosper::IDescriptorSet &descSet)
{
	return RecordBindDescriptorSet(descSet,GetInstanceDescriptorSetIndex());//,{0u,0u});
}

bool ShaderEntity::BindVertexAnimationOffset(uint32_t offset)
{
	uint32_t pushConstantOffset;
	GetVertexAnimationPushConstantInfo(pushConstantOffset);
	return RecordPushConstants(sizeof(offset),&offset,pushConstantOffset);
}

bool ShaderEntity::BindScene(pragma::CSceneComponent &scene,CRasterizationRendererComponent &renderer,bool bView)
{
	return ShaderSceneLit::BindScene(scene,renderer,bView) &&
		BindRenderSettings(c_game->GetGlobalRenderSettingsDescriptorSet());
}
bool ShaderEntity::Draw(CModelSubMesh &mesh,const std::optional<pragma::RenderMeshIndex> &meshIdx,prosper::IBuffer &renderBufferIndexBuffer,const std::function<bool(CModelSubMesh&)> &fDraw,bool bUseVertexWeightBuffer)
{
	auto numTriangleVertices = mesh.GetTriangleVertexCount();
	if(numTriangleVertices > umath::to_integral(GameLimits::MaxMeshVertices))
	{
		Con::cerr<<"ERROR: Attempted to draw mesh with more than maximum ("<<umath::to_integral(GameLimits::MaxMeshVertices)<<") amount of vertices!"<<Con::endl;
		return false;
	}
	auto &vkMesh = mesh.GetSceneMesh();
	auto &renderBuffer = vkMesh->GetRenderBuffer(mesh,*this,m_currentPipelineIdx);
	if(renderBuffer == nullptr || RecordBindRenderBuffer(*renderBuffer) == false)
		return false;
	return RecordBindVertexBuffer(renderBufferIndexBuffer,umath::to_integral(VertexBinding::RenderBufferIndex)) && fDraw(mesh);
#if 0
	auto &vertexBuffer = vkMesh->GetVertexBuffer();
	auto &indexBuffer = vkMesh->GetIndexBuffer();
	auto &vertexWeightBuffer = vkMesh->GetVertexWeightBuffer();
	if(vkMesh == nullptr || vertexBuffer == nullptr || indexBuffer == nullptr)
	{
		// TODO: Re-enable this once a logging system with categories is in place
		/*Con::cwar<<"WARNING: Attempted to render mesh with invalid ";
		if(vkMesh == nullptr)
			Con::cwar<<"VKMesh";
		else if(vertexBuffer == nullptr)
			Con::cwar<<"Vertex Buffer";
		else
			Con::cwar<<"Index Buffer";
		Con::cwar<<"! Skipping..."<<Con::endl;*/
		return false;
	}

	auto &scene = static_cast<CGame*>(c_engine->GetClientState()->GetGameState())->GetRenderScene();
	auto &cam = scene->GetActiveCamera();
	auto mvp = cam.valid() ? cam->GetProjectionMatrix() *cam->GetViewMatrix() : Mat4{1.f};
	std::vector<prosper::IBuffer*> vertexBuffers;
	std::vector<uint64_t> offsets;
	vertexBuffers.reserve(3u);
	offsets.reserve(3u);
	if(bUseVertexWeightBuffer == true)
	{
		vertexBuffers.push_back((vertexWeightBuffer != nullptr) ? vertexWeightBuffer.get() : c_engine->GetRenderContext().GetDummyBuffer().get());
		offsets.push_back(0ull);

		// Extended vertex weights
		auto &vertWeights = mesh.GetVertexWeights();
		auto offset = vertWeights.size() *sizeof(vertWeights.front());
		vertexBuffers.push_back((vertexWeightBuffer != nullptr) ? vertexWeightBuffer.get() : c_engine->GetRenderContext().GetDummyBuffer().get());
		offsets.push_back(offset);
	}
	vertexBuffers.push_back(vertexBuffer.get());
	offsets.push_back(0ull);
	return RecordBindVertexBuffers(vertexBuffers,0u,offsets) &&
		RecordBindIndexBuffer(*indexBuffer,prosper::IndexType::UInt16) &&
		fDraw(mesh);
#endif
}

bool ShaderEntity::Draw(CModelSubMesh &mesh,const std::optional<pragma::RenderMeshIndex> &meshIdx,prosper::IBuffer &renderBufferIndexBuffer,bool bUseVertexWeightBuffer,uint32_t instanceCount)
{
	return Draw(mesh,meshIdx,renderBufferIndexBuffer,[this,instanceCount](CModelSubMesh &mesh) {
#if 0
		static std::shared_ptr<prosper::IBuffer> vertexBuffer = nullptr;
		if(vertexBuffer == nullptr)
		{
			static const GLfloat g_vertex_buffer_data[] = {
				-1.0f, -1.0f, 0.0f,
				1.0f, -1.0f, 0.0f,
				0.0f,  1.0f, 0.0f,
			};
			prosper::util::BufferCreateInfo bufCreateInfo {};
			bufCreateInfo.size = sizeof(GLfloat) *9;
			bufCreateInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit;
			bufCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::DeviceLocal;
			vertexBuffer = GetContext().CreateBuffer(bufCreateInfo,g_vertex_buffer_data);
		}
		RecordBindVertexBuffer(*vertexBuffer);

		RecordDraw(9);
		return true;
#endif
		return RecordDrawIndexed(mesh.GetTriangleVertexCount(),instanceCount);
	},bUseVertexWeightBuffer);
}

bool ShaderEntity::Draw(CModelSubMesh &mesh,const std::optional<pragma::RenderMeshIndex> &meshIdx,prosper::IBuffer &renderBufferIndexBuffer,uint32_t instanceCount) {return Draw(mesh,meshIdx,renderBufferIndexBuffer,true,instanceCount);}

/////////////

void pragma::ShaderGameWorld::RecordSceneFlags(rendering::ShaderProcessor &shaderProcessor,SceneFlags sceneFlags) const
{
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(),prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit,offsetof(ScenePushConstants,flags),sizeof(sceneFlags),&sceneFlags);
}

bool pragma::ShaderGameWorld::RecordBindMaterial(rendering::ShaderProcessor &shaderProcessor,CMaterial &mat) const
{
	auto descSetGroup = mat.GetDescriptorSetGroup(const_cast<pragma::ShaderGameWorld&>(*this));
	if(descSetGroup == nullptr)
		descSetGroup = const_cast<pragma::ShaderGameWorld*>(this)->InitializeMaterialDescriptorSet(mat,false); // Attempt to initialize on the fly (TODO: Is this thread safe?)
	if(descSetGroup == nullptr)
		return false;
	shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics,shaderProcessor.GetCurrentPipelineLayout(),GetMaterialDescriptorSetIndex(),*descSetGroup->GetDescriptorSet(0));
	return true;
}

void pragma::ShaderGameWorld::RecordClipPlane(rendering::ShaderProcessor &shaderProcessor,const Vector4 &clipPlane) const
{
	shaderProcessor.GetCommandBuffer().RecordPushConstants(
		shaderProcessor.GetCurrentPipelineLayout(),prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit,offsetof(pragma::ShaderGameWorld::ScenePushConstants,clipPlane),sizeof(clipPlane),&clipPlane
	);
}
