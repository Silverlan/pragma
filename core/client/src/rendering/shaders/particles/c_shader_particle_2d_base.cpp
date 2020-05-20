/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_2d_base.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/console/c_cvar.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_util.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <prosper_descriptor_set_group.hpp>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

decltype(ShaderParticle2DBase::VERTEX_BINDING_PARTICLE) ShaderParticle2DBase::VERTEX_BINDING_PARTICLE = {prosper::VertexInputRate::Instance,sizeof(pragma::CParticleSystemComponent::ParticleData)};
decltype(ShaderParticle2DBase::VERTEX_ATTRIBUTE_POSSCALE) ShaderParticle2DBase::VERTEX_ATTRIBUTE_POSSCALE = {VERTEX_BINDING_PARTICLE,prosper::Format::R32G32B32A32_SFloat};
decltype(ShaderParticle2DBase::VERTEX_ATTRIBUTE_PREVPOS) ShaderParticle2DBase::VERTEX_ATTRIBUTE_PREVPOS = {VERTEX_BINDING_PARTICLE,prosper::Format::R32G32B32A32_SFloat};
decltype(ShaderParticle2DBase::VERTEX_ATTRIBUTE_COLOR) ShaderParticle2DBase::VERTEX_ATTRIBUTE_COLOR = {VERTEX_BINDING_PARTICLE,prosper::Format::R16G16B16A16_UNorm};
decltype(ShaderParticle2DBase::VERTEX_ATTRIBUTE_ROTATION) ShaderParticle2DBase::VERTEX_ATTRIBUTE_ROTATION = {VERTEX_BINDING_PARTICLE,prosper::Format::R32_SFloat};
decltype(ShaderParticle2DBase::VERTEX_ATTRIBUTE_LENGTH) ShaderParticle2DBase::VERTEX_ATTRIBUTE_LENGTH = {VERTEX_BINDING_PARTICLE,prosper::Format::R32_SFloat};

decltype(ShaderParticle2DBase::VERTEX_BINDING_ANIMATION_START) ShaderParticle2DBase::VERTEX_BINDING_ANIMATION_START = {prosper::VertexInputRate::Instance};
decltype(ShaderParticle2DBase::VERTEX_ATTRIBUTE_ANIMATION_START) ShaderParticle2DBase::VERTEX_ATTRIBUTE_ANIMATION_START = {VERTEX_BINDING_ANIMATION_START,prosper::Format::R32_SFloat};

decltype(ShaderParticle2DBase::DESCRIPTOR_SET_TEXTURE) ShaderParticle2DBase::DESCRIPTOR_SET_TEXTURE = {
	{
		prosper::DescriptorSetInfo::Binding {
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
decltype(ShaderParticle2DBase::DESCRIPTOR_SET_DEPTH_MAP) ShaderParticle2DBase::DESCRIPTOR_SET_DEPTH_MAP = {
	{
		prosper::DescriptorSetInfo::Binding {
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
decltype(ShaderParticle2DBase::DESCRIPTOR_SET_ANIMATION) ShaderParticle2DBase::DESCRIPTOR_SET_ANIMATION = {&ShaderParticleBase::DESCRIPTOR_SET_ANIMATION};
decltype(ShaderParticle2DBase::DESCRIPTOR_SET_CAMERA) ShaderParticle2DBase::DESCRIPTOR_SET_CAMERA = {&ShaderSceneLit::DESCRIPTOR_SET_CAMERA};
decltype(ShaderParticle2DBase::DESCRIPTOR_SET_RENDER_SETTINGS) ShaderParticle2DBase::DESCRIPTOR_SET_RENDER_SETTINGS = {&ShaderSceneLit::DESCRIPTOR_SET_RENDER_SETTINGS};
decltype(ShaderParticle2DBase::DESCRIPTOR_SET_LIGHTS) ShaderParticle2DBase::DESCRIPTOR_SET_LIGHTS = {&ShaderSceneLit::DESCRIPTOR_SET_LIGHTS};
decltype(ShaderParticle2DBase::DESCRIPTOR_SET_CSM) ShaderParticle2DBase::DESCRIPTOR_SET_CSM = {&ShaderSceneLit::DESCRIPTOR_SET_CSM};
decltype(ShaderParticle2DBase::DESCRIPTOR_SET_SHADOWS) ShaderParticle2DBase::DESCRIPTOR_SET_SHADOWS = {&ShaderSceneLit::DESCRIPTOR_SET_SHADOWS};
ShaderParticle2DBase::ShaderParticle2DBase(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: ShaderSceneLit(context,identifier,vsShader,fsShader,gsShader)
{
	SetPipelineCount(GetParticlePipelineCount());
}
void ShaderParticle2DBase::RegisterDefaultGfxPipelineVertexAttributes(prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSSCALE);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_PREVPOS);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_COLOR);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_ROTATION);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_LENGTH);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_ANIMATION_START);
}
void ShaderParticle2DBase::RegisterDefaultGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit);
}
void ShaderParticle2DBase::RegisterDefaultGfxPipelineDescriptorSetGroups(prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_TEXTURE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_DEPTH_MAP);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_ANIMATION);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CAMERA);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_RENDER_SETTINGS);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_LIGHTS);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CSM);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_SHADOWS);
}

void ShaderParticle2DBase::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	auto basePipelineIdx = GetBasePipelineIndex(pipelineIdx);
	ShaderSceneLit::InitializeGfxPipeline(pipelineInfo,basePipelineIdx);

	pipelineInfo.ToggleDepthWrites(false);

	ShaderParticleBase::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	RegisterDefaultGfxPipelineVertexAttributes(pipelineInfo);
	RegisterDefaultGfxPipelinePushConstantRanges(pipelineInfo);
	RegisterDefaultGfxPipelineDescriptorSetGroups(pipelineInfo);
}

bool ShaderParticle2DBase::BeginDraw(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &cmdBuffer,pragma::CParticleSystemComponent &pSys,Pipeline pipelineIdx,RecordFlags recordFlags)
{
	return ShaderSceneLit::BeginDraw(cmdBuffer,umath::to_integral(pipelineIdx) *umath::to_integral(AlphaMode::Count) +umath::to_integral(GetRenderAlphaMode(pSys)),recordFlags);
}

uint32_t ShaderParticle2DBase::GetRenderSettingsDescriptorSetIndex() const {return DESCRIPTOR_SET_RENDER_SETTINGS.setIndex;}
uint32_t ShaderParticle2DBase::GetLightDescriptorSetIndex() const {return DESCRIPTOR_SET_LIGHTS.setIndex;}
uint32_t ShaderParticle2DBase::GetCameraDescriptorSetIndex() const {return DESCRIPTOR_SET_CAMERA.setIndex;}

std::shared_ptr<prosper::IDescriptorSetGroup> ShaderParticle2DBase::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	auto *diffuseMap = mat.GetDiffuseMap();
	if(diffuseMap != nullptr && diffuseMap->texture != nullptr)
	{
		auto texture = std::static_pointer_cast<Texture>(diffuseMap->texture);
		if(texture->HasValidVkTexture())
		{
			auto descSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE);
			mat.SetDescriptorSetGroup(*this,descSetGroup);
			auto &descSet = *descSetGroup->GetDescriptorSet();
			descSet.SetBindingTexture(*texture->GetVkTexture(),0u);
			return descSetGroup;
		}
	}
	return nullptr;
}

void ShaderParticle2DBase::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	auto sampleCount = GetSampleCount(GetBasePipelineIndex(pipelineIdx));
	CreateCachedRenderPass<ShaderParticle2DBase>({{
		{
			RENDER_PASS_FORMAT,prosper::ImageLayout::ColorAttachmentOptimal,prosper::AttachmentLoadOp::Load,
			prosper::AttachmentStoreOp::Store,sampleCount,prosper::ImageLayout::ColorAttachmentOptimal
		},
		{ // Bloom Attachment
			RENDER_PASS_FORMAT,prosper::ImageLayout::ColorAttachmentOptimal,prosper::AttachmentLoadOp::Load,
			prosper::AttachmentStoreOp::Store,sampleCount,prosper::ImageLayout::ColorAttachmentOptimal
		},
		{
			RENDER_PASS_DEPTH_FORMAT,prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::AttachmentLoadOp::Load,
			prosper::AttachmentStoreOp::Store,sampleCount,prosper::ImageLayout::DepthStencilAttachmentOptimal
		}
	}},outRenderPass,pipelineIdx);
}

bool ShaderParticle2DBase::ShouldInitializePipeline(uint32_t pipelineIdx) {return ShaderSceneLit::ShouldInitializePipeline(GetBasePipelineIndex(pipelineIdx));}

void ShaderParticle2DBase::GetParticleSystemOrientationInfo(
	const Mat4 &matrix,const CParticleSystemComponent &particle,CParticleSystemComponent::OrientationType orientationType,Vector3 &up,Vector3 &right,
	float &nearZ,float &farZ,const Material *material,float camNearZ,float camFarZ
) const
{
	auto pTrComponent = particle.GetEntity().GetTransformComponent();
	auto rot = pTrComponent.valid() ? pTrComponent->GetOrientation() : uquat::identity();

	nearZ = camNearZ;
	farZ = camFarZ;
	if(orientationType != pragma::CParticleSystemComponent::OrientationType::World)
		right = Vector3(matrix[0][0],matrix[1][0],matrix[2][0]);
	else
	{
		right = uquat::right(rot);
		auto scale = particle.GetStaticWorldScale() *0.25f;
		if(material != nullptr)
		{
			nearZ = const_cast<Material*>(material)->GetDiffuseMap()->width *scale; // Width
			farZ = const_cast<Material*>(material)->GetDiffuseMap()->height *scale; // Height
		}
	}

	switch(orientationType)
	{
	case pragma::CParticleSystemComponent::OrientationType::World:
		up = uquat::up(rot);
		break;
	case pragma::CParticleSystemComponent::OrientationType::Upright:
		up = uquat::forward(rot);
		break;
	default:
		up = Vector3(matrix[0][1],matrix[1][1],matrix[2][1]);
	}
}

void ShaderParticle2DBase::GetParticleSystemOrientationInfo(
	const Mat4 &matrix,const pragma::CParticleSystemComponent &particle,pragma::CParticleSystemComponent::OrientationType orientationType,Vector3 &up,Vector3 &right,
	float &nearZ,float &farZ,const Material *material,const pragma::CCameraComponent *cam
) const
{
	auto camNearZ = 0.f;
	auto camFarZ = 0.f;
	if(cam != nullptr)
	{
		camNearZ = cam->GetNearZ();
		camFarZ = cam->GetFarZ();
	}
	else
	{
		camNearZ = 0.f;
		camFarZ = 0.f;
	}
	GetParticleSystemOrientationInfo(
		matrix,particle,orientationType,up,right,
		nearZ,farZ,material,camNearZ,camFarZ
	);
}

void ShaderParticle2DBase::GetParticleSystemOrientationInfo(
	const Mat4 &matrix,const pragma::CParticleSystemComponent &particle,Vector3 &up,Vector3 &right,
	float &nearZ,float &farZ,const Material *material,const pragma::CCameraComponent *cam
) const
{
	return GetParticleSystemOrientationInfo(
		matrix,particle,particle.GetOrientationType(),up,right,
		nearZ,farZ,material,cam
	);
}

prosper::DescriptorSetInfo &ShaderParticle2DBase::GetAnimationDescriptorSetInfo() const {return DESCRIPTOR_SET_ANIMATION;}
bool ShaderParticle2DBase::BindParticleMaterial(const rendering::RasterizationRenderer &renderer,const pragma::CParticleSystemComponent &ps)
{
	auto *mat = static_cast<CMaterial*>(ps.GetMaterial());
	if(mat == nullptr)
		return false;
	auto descSetGroupMat = mat->GetDescriptorSetGroup(*this);
	if(descSetGroupMat == nullptr)
		descSetGroupMat = InitializeMaterialDescriptorSet(*mat); // Attempt to initialize on the fly
	if(descSetGroupMat == nullptr)
		return false;
	auto &descSetTexture = *descSetGroupMat->GetDescriptorSet(); // prosper TODO: Use dummy descriptor set when not animated
	auto *descSetDepth = renderer.GetDepthDescriptorSet();
	if(descSetDepth == nullptr)
		return false;
	auto &animDescSet = GetAnimationDescriptorSet(const_cast<pragma::CParticleSystemComponent&>(ps));
	return RecordBindDescriptorSets({&descSetTexture,descSetDepth,&animDescSet},DESCRIPTOR_SET_TEXTURE.setIndex);
}

bool ShaderParticle2DBase::Draw(const rendering::RasterizationRenderer &renderer,const pragma::CParticleSystemComponent &ps,pragma::CParticleSystemComponent::OrientationType orientationType,bool bloom)
{
	if(BindParticleMaterial(renderer,ps) == false)
		return false;
	auto &scene = renderer.GetScene();
	auto &cam = scene.GetActiveCamera();
	auto texIntensity = (bloom == true) ? ps.GetBloomScale() : ps.GetIntensity();
	auto renderFlags = GetRenderFlags(ps);

	auto width = c_engine->GetRenderContext().GetWindowWidth();
	auto height = c_engine->GetRenderContext().GetWindowHeight();
	assert(width <= std::numeric_limits<uint16_t>::max() && height <= std::numeric_limits<uint16_t>::max());

	auto viewportSize = static_cast<uint32_t>(width);
	viewportSize<<=16;
	viewportSize |= height;
	PushConstants pushConstants {
		Vector3 {}, /* camRightWs */
		umath::to_integral(orientationType),
		Vector3{}, /* camUpWs */
		float{}, /* nearZ */
		cam.valid() ? cam->GetEntity().GetPosition() : Vector3{},
		float{}, /* farZ */
		viewportSize,
		texIntensity,
		umath::to_integral(renderFlags),
		umath::to_integral(ps.GetAlphaMode()),
		ps.GetSimulationTime()
	};
	Mat4 vp;
	if(cam.valid())
	{
		auto &v = cam->GetViewMatrix();
		vp = cam->GetProjectionMatrix() *v;
	}
	GetParticleSystemOrientationInfo(vp,ps,pushConstants.camUpWs,pushConstants.camRightWs,pushConstants.nearZ,pushConstants.farZ,ps.GetMaterial());

	if(RecordPushConstants(pushConstants) == false)
		return false;
	auto bAnimated = ((renderFlags &RenderFlags::Animated) != RenderFlags::None) ? true : false;

	auto animStartBuffer = ps.GetAnimationStartBuffer();
	if(animStartBuffer == nullptr)
		animStartBuffer = c_engine->GetRenderContext().GetDummyBuffer();
	return RecordBindVertexBuffers({ps.GetParticleBuffer().get(),animStartBuffer.get()}) == true &&
		RecordDraw(pragma::CParticleSystemComponent::VERTEX_COUNT,ps.GetRenderParticleCount()) == true;
}

