#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_glow.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include <buffers/prosper_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_util.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;

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

 // prosper TODO
#if 0
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "c_shader_glow.h"
#include "cmaterialmanager.h"
#include "pragma/model/c_side.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/rendering/uniformbinding.h"
#include <textureinfo.h>

using namespace Shader;

LINK_SHADER_TO_CLASS(Glow,glow);

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

Glow::Glow()
	: TexturedBase3D("glow","world/vs_glow","world/fs_glow")
{
	//m_bUseMultiSample = false;
}
void Glow::InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions)
{
	Base::InitializeVertexDescriptions(vertexBindingDescriptions,vertexAttributeDescriptions);
	vertexBindingDescriptions.push_back({
		static_cast<uint32_t>(TexturedBase3D::Binding::Vertex),
		sizeof(Vertex),
		Anvil::VertexInputRate::VERTEX
	});
	vertexBindingDescriptions.push_back({
		static_cast<uint32_t>(TexturedBase3D::Binding::BoneWeight),
		sizeof(VertexWeight),
		Anvil::VertexInputRate::VERTEX
	});

	vertexAttributeDescriptions.push_back({
		static_cast<uint32_t>(Location::Vertex),
		static_cast<uint32_t>(TexturedBase3D::Binding::Vertex),
		Anvil::Format::R32G32B32_SFLOAT,0
	});
	vertexAttributeDescriptions.push_back({
		static_cast<uint32_t>(Location::UV),
		static_cast<uint32_t>(TexturedBase3D::Binding::UV),
		Anvil::Format::R32G32_SFLOAT,sizeof(Vector3)
	});
	vertexAttributeDescriptions.push_back({
		static_cast<uint32_t>(Location::BoneWeightId),
		static_cast<uint32_t>(TexturedBase3D::Binding::BoneWeightId),
		vk::Format::eR32G32B32A32Sint,0
	});
	vertexAttributeDescriptions.push_back({
		static_cast<uint32_t>(Location::BoneWeight),
		static_cast<uint32_t>(TexturedBase3D::Binding::BoneWeight),
		Anvil::Format::R32G32B32A32_SFLOAT,sizeof(Vector4i)
	});
}

void Glow::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	Base::InitializePipelineLayout(context,setLayouts,pushConstants);

	pushConstants.push_back({Anvil::ShaderStageFlagBits::FRAGMENT_BIT,0,1});
	pushConstants.push_back({Anvil::ShaderStageFlagBits::VERTEX_BIT,0,17});

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::UNIFORM_BUFFER_DYNAMIC,Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT}, // Instance
		{Anvil::DescriptorType::UNIFORM_BUFFER_DYNAMIC,Anvil::ShaderStageFlagBits::VERTEX_BIT} // Bone Matrices
	}));
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT}, // Camera
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT}, // Render Settings; Unused
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // SSAO Map; Unused
	}));
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // Fog; Unused
	}));

	std::vector<Vulkan::DescriptorSetLayout::Binding> materialBindings;
	InitializeMaterialBindings(materialBindings);
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,materialBindings));
}
void Glow::InitializeShaderPipelines(const Vulkan::Context &context)
{
	Base::InitializeShaderPipelines(context);

	/*auto &instanceBuffer = CBaseEntity::GetInstanceBuffer();
	auto &boneBuffer = CBaseEntity::GetInstanceBoneBuffer();
	auto it = s_descriptorCache.find(umath::to_integral(DescSet::Instance));
	auto *pipeline = GetPipeline();
	if(it != s_descriptorCache.end())
		pipeline->SetBuffer(umath::to_integral(DescSet::Instance),it->second,umath::to_integral(Binding::Instance));
	else
	{
		s_descriptorCache.insert(decltype(s_descriptorCache)::value_type(
			umath::to_integral(DescSet::Instance),
			pipeline->SetBuffer(umath::to_integral(DescSet::Instance),instanceBuffer,umath::to_integral(Binding::Instance))
		));
	}

	it = s_descriptorCache.find(umath::to_integral(DescSet::BoneMatrix));
	if(it != s_descriptorCache.end())
		pipeline->SetBuffer(umath::to_integral(DescSet::BoneMatrix),it->second,umath::to_integral(Binding::BoneMatrix));
	else
	{
		s_descriptorCache.insert(decltype(s_descriptorCache)::value_type(
			umath::to_integral(DescSet::Instance),
			pipeline->SetBuffer(umath::to_integral(DescSet::BoneMatrix),boneBuffer,umath::to_integral(Binding::BoneMatrix))
		));
	}*/
}
void Glow::SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info)
{
	TexturedBase3D::SetupPipeline(pipelineIdx,info);
	auto *rasterizationInfo = const_cast<vk::PipelineRasterizationStateCreateInfo*>(info.pRasterizationState);
	rasterizationInfo->depthBiasEnable = true;
	rasterizationInfo->depthBiasConstantFactor = -180.f; // TODO: Why such a low value?
	rasterizationInfo->depthBiasClamp = -180.f; // TODO: What does this affect?
}
void Glow::Draw(CModelSubMesh *mesh)
{
	TexturedBase3D::Draw(mesh);
}
bool Glow::BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline)
{
	return Base3D::BeginDraw(cmdBuffer,shaderPipeline); // Skip TexturedBase3D::BeginDraw
}
void Glow::EndDraw(Vulkan::CommandBufferObject *cmdBuffer)
{
	Base3D::EndDraw(cmdBuffer); // Skip TexturedBase3D::EndDraw
}
Bool Glow::BindGlowMap(Material *mat)
{
	auto *glowMap = mat->GetGlowMap();
	if(glowMap == nullptr || glowMap->texture == nullptr)
		return false;
	auto *data = mat->GetDataBlock();
	if(data != nullptr && data->GetBool("glow_alpha_only") == true)
	{
		auto texture = std::static_pointer_cast<Texture>(glowMap->texture);
		if(Vulkan::has_alpha(texture->internalFormat) == false)
			return false;
	}
	auto &context = *m_context.get();
	auto &drawCmd = context.GetDrawCmd();
	auto &layout = GetPipeline()->GetPipelineLayout();
	auto scale = 1.f;
	if(data != nullptr)
		data->GetFloat("glow_scale",&scale);
	drawCmd->PushConstants(layout,Anvil::ShaderStageFlagBits::FRAGMENT_BIT,scale);

	//Vulkan::Std140LayoutBlockData pushConstants(17);
	//auto entMvp = depthMvp *(*ent->GetTransformationMatrix());
	//weighted = static_cast<int32_t>(IsWeighted(ent));
	//pushConstants<<entMvp<<weighted; // Vulkan TODO

	auto descSetMaterial = static_cast<CMaterial*>(mat)->GetDescriptorSet();
	if(descSetMaterial != nullptr)
		drawCmd->BindDescriptorSet(umath::to_integral(DescSet::GlowMap),layout,descSetMaterial);
	return true;
}
#endif