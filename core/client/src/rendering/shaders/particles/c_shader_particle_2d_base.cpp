#include "stdafx_client.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_2d_base.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/console/c_cvar.h"
#include <buffers/prosper_buffer.hpp>
#include <prosper_util.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <prosper_descriptor_set_group.hpp>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

decltype(ShaderParticle2DBase::VERTEX_BINDING_VERTEX) ShaderParticle2DBase::VERTEX_BINDING_VERTEX = {Anvil::VertexInputRate::VERTEX};
decltype(ShaderParticle2DBase::VERTEX_ATTRIBUTE_VERTEX) ShaderParticle2DBase::VERTEX_ATTRIBUTE_VERTEX = {VERTEX_BINDING_VERTEX,Anvil::Format::R32G32_SFLOAT};

decltype(ShaderParticle2DBase::VERTEX_BINDING_PARTICLE) ShaderParticle2DBase::VERTEX_BINDING_PARTICLE = {Anvil::VertexInputRate::INSTANCE,sizeof(pragma::CParticleSystemComponent::ParticleData)};
decltype(ShaderParticle2DBase::VERTEX_ATTRIBUTE_PARTICLE) ShaderParticle2DBase::VERTEX_ATTRIBUTE_PARTICLE = {VERTEX_BINDING_PARTICLE,Anvil::Format::R32G32B32A32_SFLOAT};
decltype(ShaderParticle2DBase::VERTEX_ATTRIBUTE_COLOR) ShaderParticle2DBase::VERTEX_ATTRIBUTE_COLOR = {VERTEX_BINDING_PARTICLE,Anvil::Format::R16G16B16A16_UNORM};
decltype(ShaderParticle2DBase::VERTEX_ATTRIBUTE_ROTATION) ShaderParticle2DBase::VERTEX_ATTRIBUTE_ROTATION = {VERTEX_BINDING_PARTICLE,Anvil::Format::R32_SFLOAT};
decltype(ShaderParticle2DBase::VERTEX_ATTRIBUTE_LENGTH) ShaderParticle2DBase::VERTEX_ATTRIBUTE_LENGTH = {VERTEX_BINDING_PARTICLE,Anvil::Format::R32_SFLOAT};

decltype(ShaderParticle2DBase::VERTEX_BINDING_ANIMATION_START) ShaderParticle2DBase::VERTEX_BINDING_ANIMATION_START = {Anvil::VertexInputRate::INSTANCE};
decltype(ShaderParticle2DBase::VERTEX_ATTRIBUTE_ANIMATION_START) ShaderParticle2DBase::VERTEX_ATTRIBUTE_ANIMATION_START = {VERTEX_BINDING_ANIMATION_START,Anvil::Format::R32_SFLOAT};

decltype(ShaderParticle2DBase::DESCRIPTOR_SET_TEXTURE) ShaderParticle2DBase::DESCRIPTOR_SET_TEXTURE = {
	{
		prosper::Shader::DescriptorSetInfo::Binding {
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
decltype(ShaderParticle2DBase::DESCRIPTOR_SET_DEPTH_MAP) ShaderParticle2DBase::DESCRIPTOR_SET_DEPTH_MAP = {
	{
		prosper::Shader::DescriptorSetInfo::Binding {
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
decltype(ShaderParticle2DBase::DESCRIPTOR_SET_ANIMATION) ShaderParticle2DBase::DESCRIPTOR_SET_ANIMATION = {&ShaderParticleBase::DESCRIPTOR_SET_ANIMATION};
decltype(ShaderParticle2DBase::DESCRIPTOR_SET_CAMERA) ShaderParticle2DBase::DESCRIPTOR_SET_CAMERA = {&ShaderSceneLit::DESCRIPTOR_SET_CAMERA};
decltype(ShaderParticle2DBase::DESCRIPTOR_SET_RENDER_SETTINGS) ShaderParticle2DBase::DESCRIPTOR_SET_RENDER_SETTINGS = {&ShaderSceneLit::DESCRIPTOR_SET_RENDER_SETTINGS};
decltype(ShaderParticle2DBase::DESCRIPTOR_SET_LIGHTS) ShaderParticle2DBase::DESCRIPTOR_SET_LIGHTS = {&ShaderSceneLit::DESCRIPTOR_SET_LIGHTS};
decltype(ShaderParticle2DBase::DESCRIPTOR_SET_CSM) ShaderParticle2DBase::DESCRIPTOR_SET_CSM = {&ShaderSceneLit::DESCRIPTOR_SET_CSM};
decltype(ShaderParticle2DBase::DESCRIPTOR_SET_SHADOWS) ShaderParticle2DBase::DESCRIPTOR_SET_SHADOWS = {&ShaderSceneLit::DESCRIPTOR_SET_SHADOWS};
ShaderParticle2DBase::ShaderParticle2DBase(prosper::Context &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: ShaderSceneLit(context,identifier,vsShader,fsShader,gsShader)
{
	SetPipelineCount(GetParticlePipelineCount());
}
void ShaderParticle2DBase::RegisterDefaultGfxPipelineVertexAttributes(Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_VERTEX);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_PARTICLE);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_COLOR);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_ROTATION);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_LENGTH);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_ANIMATION_START);
}
void ShaderParticle2DBase::RegisterDefaultGfxPipelinePushConstantRanges(Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT);
}
void ShaderParticle2DBase::RegisterDefaultGfxPipelineDescriptorSetGroups(Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
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

void ShaderParticle2DBase::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	auto basePipelineIdx = GetBasePipelineIndex(pipelineIdx);
	ShaderSceneLit::InitializeGfxPipeline(pipelineInfo,basePipelineIdx);

	pipelineInfo.toggle_depth_writes(false);

	ShaderParticleBase::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	RegisterDefaultGfxPipelineVertexAttributes(pipelineInfo);
	RegisterDefaultGfxPipelinePushConstantRanges(pipelineInfo);
	RegisterDefaultGfxPipelineDescriptorSetGroups(pipelineInfo);
}

bool ShaderParticle2DBase::BeginDraw(const std::shared_ptr<prosper::PrimaryCommandBuffer> &cmdBuffer,pragma::CParticleSystemComponent &pSys,Pipeline pipelineIdx,RecordFlags recordFlags)
{
	return ShaderSceneLit::BeginDraw(cmdBuffer,umath::to_integral(pipelineIdx) *umath::to_integral(AlphaMode::Count) +umath::to_integral(GetRenderAlphaMode(pSys)),recordFlags);
}

uint32_t ShaderParticle2DBase::GetRenderSettingsDescriptorSetIndex() const {return DESCRIPTOR_SET_RENDER_SETTINGS.setIndex;}
uint32_t ShaderParticle2DBase::GetLightDescriptorSetIndex() const {return DESCRIPTOR_SET_LIGHTS.setIndex;}
uint32_t ShaderParticle2DBase::GetCameraDescriptorSetIndex() const {return DESCRIPTOR_SET_CAMERA.setIndex;}

std::shared_ptr<prosper::DescriptorSetGroup> ShaderParticle2DBase::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	auto &dev = c_engine->GetDevice();

	auto *diffuseMap = mat.GetDiffuseMap();
	if(diffuseMap != nullptr && diffuseMap->texture != nullptr)
	{
		auto texture = std::static_pointer_cast<Texture>(diffuseMap->texture);
		if(texture->texture != nullptr)
		{
			auto descSetGroup = prosper::util::create_descriptor_set_group(dev,DESCRIPTOR_SET_TEXTURE);
			mat.SetDescriptorSetGroup(*this,descSetGroup);
			auto descSet = (*descSetGroup)->get_descriptor_set(0u);
			prosper::util::set_descriptor_set_binding_texture(*descSet,*texture->texture,0u);
			return descSetGroup;
		}
	}
	return nullptr;
}

void ShaderParticle2DBase::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	auto sampleCount = GetSampleCount(GetBasePipelineIndex(pipelineIdx));
	CreateCachedRenderPass<ShaderParticle2DBase>({{
		{
			RENDER_PASS_FORMAT,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::LOAD,
			Anvil::AttachmentStoreOp::STORE,sampleCount,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL
		},
		{ // Bloom Attachment
			RENDER_PASS_FORMAT,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::LOAD,
			Anvil::AttachmentStoreOp::STORE,sampleCount,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL
		},
		{
			RENDER_PASS_DEPTH_FORMAT,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::LOAD,
			Anvil::AttachmentStoreOp::STORE,sampleCount,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		}
	}},outRenderPass,pipelineIdx);
}

bool ShaderParticle2DBase::ShouldInitializePipeline(uint32_t pipelineIdx) {return ShaderSceneLit::ShouldInitializePipeline(GetBasePipelineIndex(pipelineIdx));}

void ShaderParticle2DBase::GetParticleSystemOrientationInfo(
	const Mat4 &matrix,const pragma::CParticleSystemComponent &particle,pragma::CParticleSystemComponent::OrientationType orientationType,Vector3 &up,Vector3 &right,
	float &nearZ,float &farZ,const Material *material,const pragma::CCameraComponent *cam
) const
{
	auto pTrComponent = particle.GetEntity().GetTransformComponent();
	auto rot = pTrComponent.valid() ? pTrComponent->GetOrientation() : uquat::identity();

	if(cam != nullptr)
	{
		nearZ = cam->GetNearZ();
		farZ = cam->GetFarZ();
	}
	else
	{
		nearZ = 0.f;
		farZ = 0.f;
	}
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
	const Mat4 &matrix,const pragma::CParticleSystemComponent &particle,Vector3 &up,Vector3 &right,
	float &nearZ,float &farZ,const Material *material,const pragma::CCameraComponent *cam
) const
{
	return GetParticleSystemOrientationInfo(
		matrix,particle,particle.GetOrientationType(),up,right,
		nearZ,farZ,material,cam
	);
}

prosper::Shader::DescriptorSetInfo &ShaderParticle2DBase::GetAnimationDescriptorSetInfo() const {return DESCRIPTOR_SET_ANIMATION;}
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
	auto &descSetTexture = *(*descSetGroupMat)->get_descriptor_set(0u); // prosper TODO: Use dummy descriptor set when not animated
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

	auto width = c_engine->GetWindowWidth();
	auto height = c_engine->GetWindowHeight();
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
		umath::to_integral(ps.GetAlphaMode())
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
		animStartBuffer = c_engine->GetDummyBuffer();
	return RecordBindVertexBuffers({&ps.GetVertexBuffer()->GetAnvilBuffer(),&ps.GetParticleBuffer()->GetAnvilBuffer(),&animStartBuffer->GetAnvilBuffer()}) == true &&
		RecordDraw(pragma::CParticleSystemComponent::VERTEX_COUNT,ps.GetRenderParticleCount()) == true;
}

