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
#include <datasystem_vector.h>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;
#pragma optimize("",off)
decltype(ShaderParticle2DBase::VERTEX_BINDING_PARTICLE) ShaderParticle2DBase::VERTEX_BINDING_PARTICLE = {prosper::VertexInputRate::Instance,sizeof(pragma::CParticleSystemComponent::ParticleData)};
decltype(ShaderParticle2DBase::VERTEX_ATTRIBUTE_POSITION) ShaderParticle2DBase::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_PARTICLE,prosper::Format::R32G32B32_SFloat};
decltype(ShaderParticle2DBase::VERTEX_ATTRIBUTE_RADIUS) ShaderParticle2DBase::VERTEX_ATTRIBUTE_RADIUS = {VERTEX_BINDING_PARTICLE,prosper::Format::R32_SFloat};
decltype(ShaderParticle2DBase::VERTEX_ATTRIBUTE_PREVPOS) ShaderParticle2DBase::VERTEX_ATTRIBUTE_PREVPOS = {VERTEX_BINDING_PARTICLE,prosper::Format::R32G32B32_SFloat};
decltype(ShaderParticle2DBase::VERTEX_ATTRIBUTE_AGE) ShaderParticle2DBase::VERTEX_ATTRIBUTE_AGE = {VERTEX_BINDING_PARTICLE,prosper::Format::R32_SFloat};
decltype(ShaderParticle2DBase::VERTEX_ATTRIBUTE_COLOR) ShaderParticle2DBase::VERTEX_ATTRIBUTE_COLOR = {VERTEX_BINDING_PARTICLE,prosper::Format::R16G16B16A16_UNorm};
decltype(ShaderParticle2DBase::VERTEX_ATTRIBUTE_ROTATION) ShaderParticle2DBase::VERTEX_ATTRIBUTE_ROTATION = {VERTEX_BINDING_PARTICLE,prosper::Format::R32_SFloat};
decltype(ShaderParticle2DBase::VERTEX_ATTRIBUTE_LENGTH_YAW) ShaderParticle2DBase::VERTEX_ATTRIBUTE_LENGTH_YAW = {VERTEX_BINDING_PARTICLE,prosper::Format::R32_UInt};

decltype(ShaderParticle2DBase::VERTEX_BINDING_ANIMATION_START) ShaderParticle2DBase::VERTEX_BINDING_ANIMATION_START = {prosper::VertexInputRate::Instance,sizeof(pragma::CParticleSystemComponent::ParticleAnimationData)};
decltype(ShaderParticle2DBase::VERTEX_ATTRIBUTE_ANIMATION_FRAME_INDICES) ShaderParticle2DBase::VERTEX_ATTRIBUTE_ANIMATION_FRAME_INDICES = {VERTEX_BINDING_ANIMATION_START,prosper::Format::R32_UInt};
decltype(ShaderParticle2DBase::VERTEX_ATTRIBUTE_ANIMATION_INTERP_FACTOR) ShaderParticle2DBase::VERTEX_ATTRIBUTE_ANIMATION_INTERP_FACTOR = {VERTEX_BINDING_ANIMATION_START,prosper::Format::R32_SFloat};

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

static uint32_t get_vertex_index(uint32_t absVertIdx)
{
	constexpr std::array<uint32_t,6> baseIndices = {0,1,2,3,0,2};
	return baseIndices.at(absVertIdx);
}
std::array<Vector2,4> ShaderParticle2DBase::GetQuadVertexPositions()
{
	return std::array<Vector2,4>{
		Vector2{0.5,-0.5},
		Vector2{-0.5,-0.5},
		Vector2{-0.5,0.5},
		Vector2{0.5,0.5}
	};
}
Vector2 ShaderParticle2DBase::GetVertexUV(uint32_t vertIdx)
{
	auto uv = GetQuadVertexPositions().at(get_vertex_index(vertIdx)) +Vector2{0.5f,0.5f};
	uv = {uv.y,1.f -uv.x};
	return uv;
}
ShaderParticle2DBase::ShaderParticle2DBase(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: ShaderSceneLit(context,identifier,vsShader,fsShader,gsShader)
{
	SetPipelineCount(GetParticlePipelineCount());
}
void ShaderParticle2DBase::RegisterDefaultGfxPipelineVertexAttributes(prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_RADIUS);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_PREVPOS);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_AGE);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_COLOR);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_ROTATION);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_LENGTH_YAW);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_ANIMATION_FRAME_INDICES);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_ANIMATION_INTERP_FACTOR);
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

	pipelineInfo.ToggleDepthWrites(pipelineIdx == GetDepthPipelineIndex()); // Last pipeline is depth pipeline

	ShaderParticleBase::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	RegisterDefaultGfxPipelineVertexAttributes(pipelineInfo);
	RegisterDefaultGfxPipelinePushConstantRanges(pipelineInfo);
	RegisterDefaultGfxPipelineDescriptorSetGroups(pipelineInfo);
}

bool ShaderParticle2DBase::BeginDraw(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &cmdBuffer,pragma::CParticleSystemComponent &pSys,ParticleRenderFlags renderFlags,Pipeline pipeline,RecordFlags recordFlags)
{
	uint32_t pipelineIdx;
	if(umath::is_flag_set(renderFlags,ParticleRenderFlags::DepthOnly))
		pipelineIdx = GetDepthPipelineIndex();
	else
	{
		auto alphaMode = GetRenderAlphaMode(pSys);
		pipelineIdx = umath::to_integral(pipeline) *umath::to_integral(ParticleAlphaMode::Count) +umath::to_integral(alphaMode);
	}
	return ShaderSceneLit::BeginDraw(cmdBuffer,pipelineIdx,recordFlags);
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
	if(pipelineIdx != GetDepthPipelineIndex())
	{
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
	else
	{
		// Depth only
		CreateCachedRenderPass<ShaderParticle2DBase>({{
			{
				RENDER_PASS_DEPTH_FORMAT,prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::AttachmentLoadOp::Load,
				prosper::AttachmentStoreOp::Store,sampleCount,prosper::ImageLayout::DepthStencilAttachmentOptimal
			}
		}},outRenderPass,pipelineIdx);
	}
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
	{
		right = Vector3(matrix[0][0],matrix[1][0],matrix[2][0]);
		uvec::normalize(&right);
	}
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
		uvec::normalize(&up);
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

bool ShaderParticle2DBase::Draw(const rendering::RasterizationRenderer &renderer,const pragma::CParticleSystemComponent &ps,pragma::CParticleSystemComponent::OrientationType orientationType,ParticleRenderFlags ptRenderFlags)
{
	if(BindParticleMaterial(renderer,ps) == false)
		return false;
	auto &scene = renderer.GetScene();
	auto &cam = scene.GetActiveCamera();

	auto colorFactor = renderer.GetScene().GetParticleSystemColorFactor();
	if(umath::is_flag_set(ptRenderFlags,ParticleRenderFlags::Bloom))
	{
		auto bloomColorFactor = ps.GetEffectiveBloomColorFactor();
		if(bloomColorFactor.has_value())
			colorFactor *= *bloomColorFactor;
	}
	else
	{
		auto *mat = ps.GetMaterial();
		auto &psColorFactor = ps.GetColorFactor();
		colorFactor *= psColorFactor;
		if(mat)
		{
			auto &data = mat->GetDataBlock();
			auto &dColorFactor = data->GetValue("color_factor");
			if(dColorFactor != nullptr && typeid(*dColorFactor) == typeid(ds::Vector4))
			{
				auto &matColorFactor = static_cast<ds::Vector4*>(dColorFactor.get())->GetValue();
				colorFactor *= matColorFactor;
			}
		}
	}

	auto renderFlags = GetRenderFlags(ps,ptRenderFlags);
	auto width = c_engine->GetRenderContext().GetWindowWidth();
	auto height = c_engine->GetRenderContext().GetWindowHeight();
	assert(width <= std::numeric_limits<uint16_t>::max() && height <= std::numeric_limits<uint16_t>::max());

	auto viewportSize = static_cast<uint32_t>(width);
	viewportSize<<=16;
	viewportSize |= height;
	PushConstants pushConstants {
		colorFactor,
		Vector3 {}, /* camRightWs */
		umath::to_integral(orientationType),
		Vector3{}, /* camUpWs */
		0.f, /* nearZ */
		cam.valid() ? cam->GetEntity().GetPosition() : Vector3{},
		0.f, /* farZ */
		viewportSize,
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
	GetParticleSystemOrientationInfo(vp,ps,pushConstants.camUpWs,pushConstants.camRightWs,pushConstants.nearZ,pushConstants.farZ,ps.GetMaterial(),cam.get());

	if(RecordPushConstants(pushConstants) == false)
		return false;
	auto bAnimated = ((renderFlags &RenderFlags::Animated) != RenderFlags::None) ? true : false;

	auto ptAnimBuffer = ps.GetParticleAnimationBuffer();
	if(ptAnimBuffer == nullptr)
		ptAnimBuffer = c_engine->GetRenderContext().GetDummyBuffer();
	return RecordBindVertexBuffers({ps.GetParticleBuffer().get(),ptAnimBuffer.get()}) == true &&
		RecordDraw(pragma::CParticleSystemComponent::VERTEX_COUNT,ps.GetRenderParticleCount()) == true;
}

static float get_particle_extent(float radius) {return sqrt(umath::pow2(radius) *2.0);}
static Mat4 get_rotation_matrix(Vector3 axis,float angle)
{
	uvec::normalize(&axis);
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0 - c;
	return Mat4(
		oc *axis.x *axis.x +c,oc *axis.x *axis.y -axis.z *s,oc *axis.z *axis.x +axis.y *s,0.0,
		oc *axis.x *axis.y +axis.z *s,oc *axis.y *axis.y +c,oc *axis.y *axis.z -axis.x *s,0.0,
		oc *axis.z *axis.x -axis.y *s,oc *axis.y *axis.z +axis.x *s,oc *axis.z *axis.z +c,0.0,
		0.0,0.0,0.0,1.0
	);
}

static Mat3 get_rotation_matrix(Vector4 q)
{
	return Mat3(
		1.0 -2.0 *umath::pow2(q.y) -2.0 *umath::pow2(q.z),2.0 *q.x *q.y +2.0 *q.z *q.w,2.0 *q.x *q.z -2.0 *q.y *q.w,
		2.0 *q.x *q.y -2.0 *q.z *q.w,1.0 -2.0 *umath::pow2(q.x) -2.0 *umath::pow2(q.z),2.0 *q.y *q.z +2.0 *q.x *q.w,
		2.0 *q.x *q.z +2.0 *q.y *q.w,2.0 *q.y *q.z -2.0 *q.x *q.w,1.0 -2.0 *umath::pow2(q.x) -2.0 *umath::pow2(q.y)
	);
}

static Vector2 get_vertex_quad_pos(uint32_t localVertIdx)
{
	constexpr std::array<Vector2,4> squareVerts = {
		Vector2{0.5,-0.5},
		Vector2{-0.5,-0.5},
		Vector2{-0.5,0.5},
		Vector2{0.5,0.5}
	};
	return squareVerts.at(localVertIdx);
}

Vector3 ShaderParticle2DBase::DoCalcVertexPosition(
	const pragma::CParticleSystemComponent &ptc,uint32_t ptIdx,uint32_t localVertIdx,
	const Vector3 &camPos,const Vector3 &camUpWs,const Vector3 &camRightWs,float nearZ,float farZ
) const
{
	auto orientation = ptc.GetOrientationType();
	auto &pt = ptc.GetRenderParticleData().at(ptc.TranslateParticleIndex(ptIdx));
	Vector3 particleCenterWs {pt.position.x,pt.position.y,pt.position.z};
	Vector2 vsize {get_particle_extent(pt.radius),get_particle_extent(pt.length)};
	Vector3 squareVert {get_vertex_quad_pos(localVertIdx),0.0};

	Vector3 right {};
	Vector3 up {};
	switch(orientation)
	{
	case pragma::CParticleSystemComponent::OrientationType::Upright:
	{
		auto dir = camUpWs; // 'camUp_ws' is the particle world-rotation if this orientation type is selected
		right = uvec::cross(normalize(particleCenterWs -camPos),dir);
		up = -dir;
		break;
	}
	case pragma::CParticleSystemComponent::OrientationType::Static:
		right = uvec::UP;
		up = camUpWs;
		break;
	case pragma::CParticleSystemComponent::OrientationType::World:
		up = -uvec::get_normal(camUpWs);
		right = -uvec::get_normal(camRightWs);
		vsize = Vector2{nearZ,farZ};
		break;
	default:
		right = camRightWs;
		up = camUpWs;
	}
	auto sv = get_rotation_matrix(Vector3{0.f,0.f,1.f},umath::deg_to_rad(pt.rotation)) *Vector4{squareVert.x,squareVert.y,squareVert.z,1};
	squareVert = {sv.x,sv.y,sv.z};
	return right *squareVert.x *vsize.x
		+up *squareVert.y *vsize.y;
}

Vector3 ShaderParticle2DBase::CalcVertexPosition(
	const pragma::CParticleSystemComponent &ptc,uint32_t ptIdx,uint32_t absVertIdx,
	const Vector3 &camPos,const Vector3 &camUpWs,const Vector3 &camRightWs,float nearZ,float farZ
) const
{
	return DoCalcVertexPosition(ptc,ptIdx,get_vertex_index(absVertIdx),camPos,camUpWs,camRightWs,nearZ,farZ);
}
#pragma optimize("",on)
