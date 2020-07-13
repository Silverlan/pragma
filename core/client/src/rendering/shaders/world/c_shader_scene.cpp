/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
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
		}
	}
};
decltype(ShaderScene::DESCRIPTOR_SET_CAMERA) ShaderScene::DESCRIPTOR_SET_CAMERA = {
	{
		prosper::DescriptorSetInfo::Binding { // Camera
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::GeometryBit
		},
		prosper::DescriptorSetInfo::Binding { // Render Settings
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::GeometryBit
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
	SetPipelineCount(umath::to_integral(Pipeline::Count));
}
bool ShaderScene::ShouldInitializePipeline(uint32_t pipelineIdx)
{
	if(static_cast<Pipeline>(pipelineIdx) != Pipeline::MultiSample)
		return true;
	return RENDER_PASS_SAMPLES != prosper::SampleCountFlags::e1Bit;
}
prosper::SampleCountFlags ShaderScene::GetSampleCount(uint32_t pipelineIdx) const
{
	return (static_cast<Pipeline>(pipelineIdx) == Pipeline::MultiSample) ? RENDER_PASS_SAMPLES : prosper::SampleCountFlags::e1Bit;
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
bool ShaderScene::BindSceneCamera(const rendering::RasterizationRenderer &renderer,bool bView)
{
	auto &scene = renderer.GetScene();
	auto *descSet = (bView == true) ? scene.GetViewCameraDescriptorSet() : scene.GetCameraDescriptorSetGraphics();
	return RecordBindDescriptorSet(*descSet,GetCameraDescriptorSetIndex());
}

/////////////////////

decltype(ShaderSceneLit::DESCRIPTOR_SET_LIGHTS) ShaderSceneLit::DESCRIPTOR_SET_LIGHTS = {
	{
		prosper::DescriptorSetInfo::Binding { // Light sources
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Visible light index buffer
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit
		},
		prosper::DescriptorSetInfo::Binding { // Shadow buffers
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
decltype(ShaderSceneLit::DESCRIPTOR_SET_CSM) ShaderSceneLit::DESCRIPTOR_SET_CSM = {
	{
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
bool ShaderSceneLit::BindLights(prosper::IDescriptorSet &descSetShadowMaps,prosper::IDescriptorSet &descSetLightSources)
{
	auto *descSetShadow = pragma::CShadowComponent::GetDescriptorSet();
	if(descSetShadow == nullptr)
		return false;
	return RecordBindDescriptorSets({
		&descSetLightSources,&descSetShadowMaps,descSetShadow
	},GetLightDescriptorSetIndex());
}
bool ShaderSceneLit::BindScene(rendering::RasterizationRenderer &renderer,bool bView)
{
	return BindSceneCamera(renderer,bView) &&
		BindLights(*renderer.GetCSMDescriptorSet(),*renderer.GetForwardPlusInstance().GetDescriptorSetGraphics());
}

/////////////////////

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
			prosper::DescriptorType::UniformBufferDynamic,
			prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit
		},
		prosper::DescriptorSetInfo::Binding { // Bone Matrices
			prosper::DescriptorType::UniformBufferDynamic,
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

bool ShaderEntity::BindEntity(CBaseEntity &ent)
{
	auto pRenderComponent = ent.GetRenderComponent();
	if(pRenderComponent.expired())
		return false;
	m_boundEntity = &ent;
	auto *descSet = pRenderComponent->GetRenderDescriptorSet();
	if(descSet == nullptr)
	{
		// Con::cwar<<"WARNING: Attempted to render entity "<<ent.GetClass()<<", but it has an invalid render descriptor set! Skipping..."<<Con::endl;
		return false;
	}
	//if(pRenderComponent->GetLastRenderFrame() != c_engine->GetRenderContext().GetLastFrameId())
	//	Con::cwar<<"WARNING: Entity buffer data for entity "<<ent.GetClass()<<" ("<<ent.GetIndex()<<") hasn't been updated for this frame, but entity is used in rendering! This may cause rendering issues!"<<Con::endl;
	return BindInstanceDescriptorSet(*descSet);
}

void ShaderEntity::EndDraw()
{
	ShaderSceneLit::EndDraw();
	m_boundEntity = nullptr;
}

CBaseEntity *ShaderEntity::GetBoundEntity() {return m_boundEntity;}

bool ShaderEntity::BindInstanceDescriptorSet(prosper::IDescriptorSet &descSet)
{
	return RecordBindDescriptorSet(descSet,GetInstanceDescriptorSetIndex(),{0u,0u});
}

bool ShaderEntity::BindVertexAnimationOffset(uint32_t offset)
{
	uint32_t pushConstantOffset;
	GetVertexAnimationPushConstantInfo(pushConstantOffset);
	return RecordPushConstants(sizeof(offset),&offset,pushConstantOffset);
}

bool ShaderEntity::BindScene(rendering::RasterizationRenderer &renderer,bool bView)
{
	return ShaderSceneLit::BindScene(renderer,bView) &&
		BindRenderSettings(c_game->GetGlobalRenderSettingsDescriptorSet());
}

bool ShaderEntity::Draw(CModelSubMesh &mesh,const std::function<bool(CModelSubMesh&)> &fDraw,bool bUseVertexWeightBuffer)
{
	auto numTriangleVertices = mesh.GetTriangleVertexCount();
	if(numTriangleVertices > umath::to_integral(GameLimits::MaxMeshVertices))
	{
		Con::cerr<<"ERROR: Attempted to draw mesh with more than maximum ("<<umath::to_integral(GameLimits::MaxMeshVertices)<<") amount of vertices!"<<Con::endl;
		return false;
	}
	auto &vkMesh = mesh.GetVKMesh();
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
}

bool ShaderEntity::Draw(CModelSubMesh &mesh,bool bUseVertexWeightBuffer)
{
	return Draw(mesh,[this](CModelSubMesh &mesh) {
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
		return RecordDrawIndexed(mesh.GetTriangleVertexCount());
	},bUseVertexWeightBuffer);
}

bool ShaderEntity::Draw(CModelSubMesh &mesh) {return Draw(mesh,true);}
