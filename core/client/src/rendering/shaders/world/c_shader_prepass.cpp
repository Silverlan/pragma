/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <pragma/model/vertex.h>
#include <prosper_command_buffer.hpp>
#include <prosper_util.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;
#pragma optimize("",off)
decltype(ShaderPrepassBase::VERTEX_BINDING_RENDER_BUFFER_INDEX) ShaderPrepassBase::VERTEX_BINDING_RENDER_BUFFER_INDEX = {prosper::VertexInputRate::Instance};
decltype(ShaderPrepassBase::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX) ShaderPrepassBase::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX = {ShaderEntity::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX,VERTEX_BINDING_RENDER_BUFFER_INDEX};

decltype(ShaderPrepassBase::VERTEX_BINDING_BONE_WEIGHT) ShaderPrepassBase::VERTEX_BINDING_BONE_WEIGHT = {prosper::VertexInputRate::Vertex};
decltype(ShaderPrepassBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID) ShaderPrepassBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID,VERTEX_BINDING_BONE_WEIGHT};
decltype(ShaderPrepassBase::VERTEX_ATTRIBUTE_BONE_WEIGHT) ShaderPrepassBase::VERTEX_ATTRIBUTE_BONE_WEIGHT = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT,VERTEX_BINDING_BONE_WEIGHT};

decltype(ShaderPrepassBase::VERTEX_BINDING_BONE_WEIGHT_EXT) ShaderPrepassBase::VERTEX_BINDING_BONE_WEIGHT_EXT = {prosper::VertexInputRate::Vertex};
decltype(ShaderPrepassBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID) ShaderPrepassBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID,VERTEX_BINDING_BONE_WEIGHT_EXT};
decltype(ShaderPrepassBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT) ShaderPrepassBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT,VERTEX_BINDING_BONE_WEIGHT_EXT};

decltype(ShaderPrepassBase::VERTEX_BINDING_VERTEX) ShaderPrepassBase::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex,sizeof(VertexBufferData)};
decltype(ShaderPrepassBase::VERTEX_ATTRIBUTE_POSITION) ShaderPrepassBase::VERTEX_ATTRIBUTE_POSITION = {ShaderEntity::VERTEX_ATTRIBUTE_POSITION,VERTEX_BINDING_VERTEX};
decltype(ShaderPrepassBase::VERTEX_ATTRIBUTE_UV) ShaderPrepassBase::VERTEX_ATTRIBUTE_UV = {ShaderEntity::VERTEX_ATTRIBUTE_UV,VERTEX_BINDING_VERTEX};

decltype(ShaderPrepassBase::DESCRIPTOR_SET_INSTANCE) ShaderPrepassBase::DESCRIPTOR_SET_INSTANCE = {&ShaderEntity::DESCRIPTOR_SET_INSTANCE};
decltype(ShaderPrepassBase::DESCRIPTOR_SET_SCENE) ShaderPrepassBase::DESCRIPTOR_SET_SCENE = {&ShaderScene::DESCRIPTOR_SET_SCENE};
decltype(ShaderPrepassBase::DESCRIPTOR_SET_MATERIAL) ShaderPrepassBase::DESCRIPTOR_SET_MATERIAL = {
	{
		prosper::DescriptorSetInfo::Binding { // Skybox Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
decltype(ShaderPrepassBase::DESCRIPTOR_SET_RENDER_SETTINGS) ShaderPrepassBase::DESCRIPTOR_SET_RENDER_SETTINGS = {&ShaderScene::DESCRIPTOR_SET_RENDER_SETTINGS};

prosper::util::RenderPassCreateInfo::AttachmentInfo ShaderPrepassBase::get_depth_render_pass_attachment_info(prosper::SampleCountFlags sampleCount)
{
	return prosper::util::RenderPassCreateInfo::AttachmentInfo {
		ShaderGameWorldLightingPass::RENDER_PASS_DEPTH_FORMAT,prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::AttachmentLoadOp::Clear,
		prosper::AttachmentStoreOp::Store,sampleCount,prosper::ImageLayout::DepthStencilAttachmentOptimal
	};
}

ShaderPrepassBase::ShaderPrepassBase(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader)
	: ShaderGameWorld(context,identifier,vsShader,fsShader)
{
	//SetPipelineCount(umath::to_integral(Pipeline::Count));
}
ShaderPrepassBase::ShaderPrepassBase(prosper::IPrContext &context,const std::string &identifier)
	: ShaderGameWorld(context,identifier,"world/prepass/vs_prepass_depth","")
{
	//SetPipelineCount(umath::to_integral(Pipeline::Count));
}

void ShaderPrepassBase::OnPipelinesInitialized()
{
	ShaderGameWorld::OnPipelinesInitialized();
	m_defaultMatDsg = c_engine->GetRenderContext().CreateDescriptorSetGroup(DESCRIPTOR_SET_MATERIAL);
}

bool ShaderPrepassBase::BeginDraw(
	const std::shared_ptr<prosper::ICommandBuffer> &cmdBuffer,const Vector4 &clipPlane,const Vector4 &drawOrigin,
	RecordFlags recordFlags
)
{
	if(m_dummyMaterialDsg == nullptr)
		m_dummyMaterialDsg = CreateDescriptorSetGroup(GetMaterialDescriptorSetIndex());
	Set3DSky(false);
	return ShaderScene::BeginDraw(cmdBuffer,0u,recordFlags) == true &&
		BindClipPlane(clipPlane) == true &&
		RecordPushConstants(drawOrigin,offsetof(PushConstants,drawOrigin)) &&
		RecordPushConstants(Vector2{},offsetof(PushConstants,depthBias)) &&
		RecordBindDescriptorSet(*m_dummyMaterialDsg->GetDescriptorSet(),GetMaterialDescriptorSetIndex()) &&
		// RecordPushConstants(pragma::SceneDebugMode::None,offsetof(PushConstants,debugMode)) &&
		cmdBuffer->RecordSetDepthBias() == true;
}

bool ShaderPrepassBase::BindScene(pragma::CSceneComponent &scene,CRasterizationRendererComponent &renderer,bool bView)
{
	return BindSceneCamera(scene,renderer,bView) && BindRenderSettings(c_game->GetGlobalRenderSettingsDescriptorSet());
}
bool ShaderPrepassBase::BindClipPlane(const Vector4 &clipPlane) {return RecordPushConstants(clipPlane);}
bool ShaderPrepassBase::BindDrawOrigin(const Vector4 &drawOrigin) {return RecordPushConstants(drawOrigin,offsetof(PushConstants,drawOrigin));}
bool ShaderPrepassBase::SetDepthBias(const Vector2 &depthBias) {return RecordPushConstants(depthBias,offsetof(PushConstants,depthBias));}

void ShaderPrepassBase::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderPrepassBase>({{get_depth_render_pass_attachment_info(GetSampleCount(pipelineIdx))}},outRenderPass,pipelineIdx);
}

void ShaderPrepassBase::Set3DSky(bool is3dSky) {umath::set_flag(m_sceneFlags,SceneFlags::RenderAs3DSky,is3dSky);}

std::shared_ptr<prosper::IDescriptorSetGroup> ShaderPrepassBase::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	auto *diffuseMap = mat.GetDiffuseMap();
	if(diffuseMap == nullptr || diffuseMap->texture == nullptr)
		return nullptr;
	auto diffuseTexture = std::static_pointer_cast<Texture>(diffuseMap->texture);
	if(diffuseTexture->HasValidVkTexture() == false)
		return nullptr;
	auto descSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(DESCRIPTOR_SET_MATERIAL);
	mat.SetDescriptorSetGroup(*this,descSetGroup);
	auto &descSet = *descSetGroup->GetDescriptorSet();
	descSet.SetBindingTexture(*diffuseTexture->GetVkTexture(),umath::to_integral(MaterialBinding::AlbedoMap));
	descSet.Update();
	return descSetGroup;
}

bool ShaderPrepassBase::BindMaterial(CMaterial &mat)
{
	m_alphaCutoff = {};
	auto alphaMode = mat.GetAlphaMode();
	if(alphaMode == AlphaMode::Opaque)
	{
		// We don't need this material
		return true;
	}
	auto descSetGroup = mat.GetDescriptorSetGroup(*this);
	if(descSetGroup == nullptr)
		descSetGroup = InitializeMaterialDescriptorSet(mat); // Attempt to initialize on the fly
	if(descSetGroup == nullptr)
		return false;
	if(RecordBindDescriptorSet(*descSetGroup->GetDescriptorSet(),GetMaterialDescriptorSetIndex()) == false)
		return false;
	m_alphaCutoff = (alphaMode == AlphaMode::Mask) ? mat.GetAlphaCutoff() : 0.5f;
	return true;
}

bool ShaderPrepassBase::Draw(CModelSubMesh &mesh,const std::optional<pragma::RenderMeshIndex> &meshIdx,prosper::IBuffer &renderBufferIndexBuffer,uint32_t instanceCount)
{
	auto flags = SceneFlags::None;
	if(mesh.GetExtendedVertexWeights().empty() == false)
		flags |= SceneFlags::UseExtendedVertexWeights;
	if(umath::is_flag_set(m_sceneFlags,SceneFlags::RenderAs3DSky))
		flags |= SceneFlags::RenderAs3DSky;
	if(m_alphaCutoff.has_value())
	{
		flags |= SceneFlags::AlphaTest;
		if(RecordPushConstants(*m_alphaCutoff,offsetof(PushConstants,alphaCutoff)) == false)
			return false;
	}
	return RecordPushConstants(flags,offsetof(PushConstants,flags)) && ShaderEntity::Draw(mesh,meshIdx,renderBufferIndexBuffer,instanceCount);
}

void ShaderPrepassBase::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderEntity::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	//if(pipelineIdx == umath::to_integral(Pipeline::Reflection))
	//	prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo,prosper::CullModeFlags::FrontBit);

	pipelineInfo.ToggleDepthWrites(true);
	pipelineInfo.ToggleDepthTest(true,prosper::CompareOp::Less);

	//pipelineInfo.ToggleDepthBias(true,0.f,0.f,0.f);
	//pipelineInfo.ToggleDynamicState(true,prosper::DynamicState::DepthBias); // Required for decals
	
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT_ID);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_UV);

	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_INSTANCE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_MATERIAL);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_SCENE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_RENDER_SETTINGS);
}

uint32_t ShaderPrepassBase::GetCameraDescriptorSetIndex() const {return DESCRIPTOR_SET_SCENE.setIndex;}
uint32_t ShaderPrepassBase::GetInstanceDescriptorSetIndex() const {return DESCRIPTOR_SET_INSTANCE.setIndex;}
uint32_t ShaderPrepassBase::GetMaterialDescriptorSetIndex() const {return DESCRIPTOR_SET_MATERIAL.setIndex;}
uint32_t ShaderPrepassBase::GetRenderSettingsDescriptorSetIndex() const {return DESCRIPTOR_SET_RENDER_SETTINGS.setIndex;}
void ShaderPrepassBase::GetVertexAnimationPushConstantInfo(uint32_t &offset) const
{
	offset = offsetof(PushConstants,vertexAnimInfo);
}

//

void ShaderPrepassBase::RecordBindScene(
	rendering::ShaderProcessor &shaderProcessor,
	const pragma::CSceneComponent &scene,const pragma::CRasterizationRendererComponent &renderer,
	prosper::IDescriptorSet &dsScene,prosper::IDescriptorSet &dsRenderer,
	prosper::IDescriptorSet &dsRenderSettings,prosper::IDescriptorSet &dsLights,
	prosper::IDescriptorSet &dsShadows,prosper::IDescriptorSet &dsMaterial,
	ShaderGameWorld::SceneFlags &inOutSceneFlags
) const
{
	std::array<prosper::IDescriptorSet*,3> descSets {
		&dsMaterial,
		&dsScene,
		&dsRenderSettings
	};

	ShaderPrepass::PushConstants pushConstants {};
	pushConstants.Initialize();
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(),prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit,0u,sizeof(pushConstants),&pushConstants);

	static const std::vector<uint32_t> dynamicOffsets {};
	shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics,shaderProcessor.GetCurrentPipelineLayout(),pragma::ShaderGameWorld::MATERIAL_DESCRIPTOR_SET_INDEX,descSets,dynamicOffsets);
}

void ShaderPrepassBase::RecordAlphaCutoff(rendering::ShaderProcessor &shaderProcessor,float alphaCutoff) const
{
	shaderProcessor.GetCommandBuffer().RecordPushConstants(
		shaderProcessor.GetCurrentPipelineLayout(),prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit,offsetof(PushConstants,alphaCutoff),sizeof(alphaCutoff),&alphaCutoff
	);
}

bool ShaderPrepassBase::RecordBindMaterial(rendering::ShaderProcessor &shaderProcessor,CMaterial &mat) const
{
	if(mat.GetAlphaMode() == AlphaMode::Opaque)
		return false;
	return ShaderGameWorld::RecordBindMaterial(shaderProcessor,mat);
}

//////////////////

decltype(ShaderPrepass::VERTEX_ATTRIBUTE_NORMAL) ShaderPrepass::VERTEX_ATTRIBUTE_NORMAL = {ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_NORMAL,VERTEX_BINDING_VERTEX};

decltype(ShaderPrepass::RENDER_PASS_NORMAL_FORMAT) ShaderPrepass::RENDER_PASS_NORMAL_FORMAT = prosper::Format::R16G16B16A16_SFloat;

prosper::util::RenderPassCreateInfo::AttachmentInfo ShaderPrepass::get_normal_render_pass_attachment_info(prosper::SampleCountFlags sampleCount)
{
	return prosper::util::RenderPassCreateInfo::AttachmentInfo {
		RENDER_PASS_NORMAL_FORMAT,prosper::ImageLayout::ColorAttachmentOptimal,prosper::AttachmentLoadOp::DontCare,
		prosper::AttachmentStoreOp::Store,sampleCount,prosper::ImageLayout::ColorAttachmentOptimal
	};
}

ShaderPrepass::ShaderPrepass(prosper::IPrContext &context,const std::string &identifier)
	: ShaderPrepassBase(context,identifier,"world/prepass/vs_prepass","world/prepass/fs_prepass")
{
	// SetBaseShader<ShaderTextured3DBase>();
	SetPipelineCount(umath::to_integral(Pipeline::Count));
}

void ShaderPrepass::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	auto sampleCount = GetSampleCount(pipelineIdx);
	prosper::util::RenderPassCreateInfo rpInfo {{
		get_normal_render_pass_attachment_info(sampleCount),
		get_depth_render_pass_attachment_info(sampleCount)
	}};
	CreateCachedRenderPass<ShaderPrepass>({rpInfo},outRenderPass,pipelineIdx);
}

void ShaderPrepass::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderPrepassBase::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	
	auto enableAlphaTest = false;
	auto enableNormalOutput = false;
	auto enableAnimation = false;
	auto enableMorphTargetAnimation = false;
	auto extendedVertexWeights = false;
	switch(static_cast<Pipeline>(pipelineIdx))
	{
	case Pipeline::Opaque:
		enableAnimation = true;
		enableMorphTargetAnimation = true;
		break;
	case Pipeline::AlphaTest:
		enableAlphaTest = true;
		
		enableAnimation = true;
		enableMorphTargetAnimation = true;
		break;
	case Pipeline::AnimatedOpaque:
		enableAnimation = true;
		enableMorphTargetAnimation = true;
		break;
	case Pipeline::AnimatedAlphaTest:
		enableAlphaTest = true;
		enableAnimation = true;
		enableMorphTargetAnimation = true;
		break;
	}
	if(c_game->GetGameWorldShaderSettings().ssaoEnabled)
		enableNormalOutput = true;
	AddSpecializationConstant(pipelineInfo,prosper::ShaderStageFlags::FragmentBit,umath::to_integral(SpecializationConstant::EnableAlphaTest),static_cast<uint32_t>(enableAlphaTest));
	AddSpecializationConstant(pipelineInfo,prosper::ShaderStageFlags::FragmentBit,umath::to_integral(SpecializationConstant::EnableNormalOutput),static_cast<uint32_t>(enableNormalOutput));
	AddSpecializationConstant(pipelineInfo,prosper::ShaderStageFlags::VertexBit,umath::to_integral(SpecializationConstant::EnableAnimation),static_cast<uint32_t>(enableAnimation));
	AddSpecializationConstant(pipelineInfo,prosper::ShaderStageFlags::VertexBit,umath::to_integral(SpecializationConstant::EnableMorphTargetAnimation),static_cast<uint32_t>(enableMorphTargetAnimation));
	AddSpecializationConstant(pipelineInfo,prosper::ShaderStageFlags::VertexBit,umath::to_integral(SpecializationConstant::EnableExtendedVertexWeights),static_cast<uint32_t>(extendedVertexWeights));

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_NORMAL);
}
#pragma optimize("",on)
