#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_glow.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include <buffers/prosper_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_util.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;

#pragma optimize("",off)
decltype(ShaderGlow::DESCRIPTOR_SET_INSTANCE) ShaderGlow::DESCRIPTOR_SET_INSTANCE = {&ShaderTextured3DBase::DESCRIPTOR_SET_INSTANCE};
decltype(ShaderGlow::DESCRIPTOR_SET_CAMERA) ShaderGlow::DESCRIPTOR_SET_CAMERA = {&ShaderTextured3DBase::DESCRIPTOR_SET_CAMERA};
decltype(ShaderGlow::DESCRIPTOR_SET_MATERIAL) ShaderGlow::DESCRIPTOR_SET_MATERIAL = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Glow Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
decltype(ShaderGlow::RENDER_PASS_FORMAT) ShaderGlow::RENDER_PASS_FORMAT = Anvil::Format::R8G8B8A8_UNORM;
ShaderGlow::ShaderGlow(prosper::Context &context,const std::string &identifier)
	: ShaderTextured3DBase(context,identifier,"world/vs_glow","world/fs_glow")
{
	SetBaseShader<ShaderTextured3D>();
}
prosper::Shader::DescriptorSetInfo &ShaderGlow::GetMaterialDescriptorSetInfo() const {return DESCRIPTOR_SET_MATERIAL;}
void ShaderGlow::InitializeGfxPipelinePushConstantRanges(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),Anvil::ShaderStageFlagBits::FRAGMENT_BIT);
}
void ShaderGlow::InitializeGfxPipelineDescriptorSets(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_INSTANCE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CAMERA);
	AddDescriptorSetGroup(pipelineInfo,GetMaterialDescriptorSetInfo());
}
void ShaderGlow::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderTextured3DBase::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	pipelineInfo.toggle_depth_bias(true,-180.f /* constant factor */,-180.f /* clamp */,0.f /* slope factor */);
}
bool ShaderGlow::BeginDraw(const std::shared_ptr<prosper::PrimaryCommandBuffer> &cmdBuffer,Pipeline pipelineIdx)
{
	return ShaderSceneLit::BeginDraw(cmdBuffer,umath::to_integral(pipelineIdx));
}
uint32_t ShaderGlow::GetCameraDescriptorSetIndex() const {return DESCRIPTOR_SET_CAMERA.setIndex;}
uint32_t ShaderGlow::GetInstanceDescriptorSetIndex() const {return DESCRIPTOR_SET_INSTANCE.setIndex;}
void ShaderGlow::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	auto sampleCount = GetSampleCount(pipelineIdx);
	CreateCachedRenderPass<ShaderGlow>({{{
		{
			RENDER_PASS_FORMAT,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::CLEAR,
			Anvil::AttachmentStoreOp::STORE,sampleCount,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL
		},
		{
			RENDER_PASS_DEPTH_FORMAT,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::LOAD,
			Anvil::AttachmentStoreOp::STORE /* depth values have already been written by prepass */,sampleCount,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		}
	}}},outRenderPass,pipelineIdx);
}
std::shared_ptr<prosper::DescriptorSetGroup> ShaderGlow::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	auto &dev = c_engine->GetDevice();
	auto *glowMap = mat.GetGlowMap();
	if(glowMap == nullptr || glowMap->texture == nullptr)
		return nullptr;
	auto glowTexture = std::static_pointer_cast<Texture>(glowMap->texture);
	if(glowTexture->HasValidVkTexture() == false)
		return nullptr;
	auto descSetGroup = prosper::util::create_descriptor_set_group(dev,DESCRIPTOR_SET_MATERIAL);
	mat.SetDescriptorSetGroup(*this,descSetGroup);
	auto &descSet = *descSetGroup->GetDescriptorSet();
	prosper::util::set_descriptor_set_binding_texture(descSet,*glowTexture->GetVkTexture(),0u);
	return descSetGroup;
}
bool ShaderGlow::BindClipPlane(const Vector4 &clipPlane)
{
	// Clip plane currently not supported for glow shaders
	return true;
}
bool ShaderGlow::BindGlowMaterial(CMaterial &mat)
{
	auto *glowMap = mat.GetGlowMap();
	if(glowMap == nullptr || glowMap->texture == nullptr)
		return false;
	auto descSetGroup = mat.GetDescriptorSetGroup(*this);
	if(descSetGroup == nullptr)
		descSetGroup = InitializeMaterialDescriptorSet(mat); // Attempt to initialize on the fly
	if(descSetGroup == nullptr)
		return false;
	auto &data = mat.GetDataBlock();
	if(data != nullptr && data->GetBool("glow_alpha_only") == true)
	{
		auto texture = std::static_pointer_cast<Texture>(glowMap->texture);
		if(prosper::util::has_alpha(texture->GetVkTexture()->GetImage()->GetFormat()) == false)
			return false;
	}
	auto scale = 1.f;
	if(data != nullptr)
		data->GetFloat("glow_scale",&scale);

	return RecordPushConstants(PushConstants{scale}) &&
		RecordBindDescriptorSet(*(*descSetGroup)->get_descriptor_set(0u),GetMaterialDescriptorSetIndex());
}
#pragma optimize("",on)
