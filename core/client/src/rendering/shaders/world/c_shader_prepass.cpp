#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include <pragma/model/vertex.h>
#include <prosper_util.hpp>

using namespace pragma;

ShaderPrepassBase::Pipeline ShaderPrepassBase::GetPipelineIndex(Anvil::SampleCountFlagBits sampleCount)
{
	return (sampleCount == Anvil::SampleCountFlagBits::_1_BIT) ? Pipeline::Regular : Pipeline::MultiSample;
}

decltype(ShaderPrepassBase::VERTEX_BINDING_BONE_WEIGHT) ShaderPrepassBase::VERTEX_BINDING_BONE_WEIGHT = {Anvil::VertexInputRate::VERTEX};
decltype(ShaderPrepassBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID) ShaderPrepassBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID,VERTEX_BINDING_BONE_WEIGHT};
decltype(ShaderPrepassBase::VERTEX_ATTRIBUTE_BONE_WEIGHT) ShaderPrepassBase::VERTEX_ATTRIBUTE_BONE_WEIGHT = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT,VERTEX_BINDING_BONE_WEIGHT};

decltype(ShaderPrepassBase::VERTEX_BINDING_VERTEX) ShaderPrepassBase::VERTEX_BINDING_VERTEX = {Anvil::VertexInputRate::VERTEX,sizeof(Vertex)};
decltype(ShaderPrepassBase::VERTEX_ATTRIBUTE_POSITION) ShaderPrepassBase::VERTEX_ATTRIBUTE_POSITION = {ShaderEntity::VERTEX_ATTRIBUTE_POSITION,VERTEX_BINDING_VERTEX};

decltype(ShaderPrepassBase::DESCRIPTOR_SET_INSTANCE) ShaderPrepassBase::DESCRIPTOR_SET_INSTANCE = {&ShaderEntity::DESCRIPTOR_SET_INSTANCE};
decltype(ShaderPrepassBase::DESCRIPTOR_SET_CAMERA) ShaderPrepassBase::DESCRIPTOR_SET_CAMERA = {&ShaderScene::DESCRIPTOR_SET_CAMERA};

static prosper::util::RenderPassCreateInfo::AttachmentInfo get_depth_render_pass_attachment_info(Anvil::SampleCountFlagBits sampleCount)
{
	return prosper::util::RenderPassCreateInfo::AttachmentInfo {
		ShaderTextured3DBase::RENDER_PASS_DEPTH_FORMAT,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::CLEAR,
		Anvil::AttachmentStoreOp::STORE,sampleCount,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};
}

ShaderPrepassBase::ShaderPrepassBase(prosper::Context &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader)
	: ShaderEntity(context,identifier,vsShader,fsShader)
{
	SetPipelineCount(umath::to_integral(Pipeline::Count));
}
ShaderPrepassBase::ShaderPrepassBase(prosper::Context &context,const std::string &identifier)
	: ShaderEntity(context,identifier,"world/prepass/vs_prepass_depth","")
{
	SetPipelineCount(umath::to_integral(Pipeline::Count));
}

bool ShaderPrepassBase::BeginDraw(const std::shared_ptr<prosper::PrimaryCommandBuffer> &cmdBuffer,Pipeline pipelineIdx)
{
	return ShaderEntity::BeginDraw(cmdBuffer,umath::to_integral(pipelineIdx));
}

bool ShaderPrepassBase::BindClipPlane(const Vector4 &clipPlane)
{
	return RecordPushConstants(clipPlane);
}

void ShaderPrepassBase::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderPrepassBase>({{get_depth_render_pass_attachment_info(GetSampleCount(pipelineIdx))}},outRenderPass,pipelineIdx);
}

void ShaderPrepassBase::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderEntity::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	if(pipelineIdx == umath::to_integral(Pipeline::Reflection))
		prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo,Anvil::CullModeFlagBits::FRONT_BIT);

	pipelineInfo.toggle_depth_writes(true);
	pipelineInfo.toggle_depth_test(true,Anvil::CompareOp::LESS);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT_ID);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);

	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),Anvil::ShaderStageFlagBits::VERTEX_BIT);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_INSTANCE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CAMERA);
}

uint32_t ShaderPrepassBase::GetCameraDescriptorSetIndex() const {return DESCRIPTOR_SET_CAMERA.setIndex;}
uint32_t ShaderPrepassBase::GetInstanceDescriptorSetIndex() const {return DESCRIPTOR_SET_INSTANCE.setIndex;}
void ShaderPrepassBase::GetVertexAnimationPushConstantInfo(uint32_t &offset) const
{
	offset = offsetof(PushConstants,vertexAnimInfo);
}

//////////////////

decltype(ShaderPrepass::VERTEX_ATTRIBUTE_NORMAL) ShaderPrepass::VERTEX_ATTRIBUTE_NORMAL = {ShaderTextured3DBase::VERTEX_ATTRIBUTE_NORMAL,VERTEX_BINDING_VERTEX};

decltype(ShaderPrepass::RENDER_PASS_NORMAL_FORMAT) ShaderPrepass::RENDER_PASS_NORMAL_FORMAT = Anvil::Format::R16G16B16A16_SFLOAT;
ShaderPrepass::ShaderPrepass(prosper::Context &context,const std::string &identifier)
	: ShaderPrepassBase(context,identifier,"world/prepass/vs_prepass","world/prepass/fs_prepass")
{
	SetBaseShader<ShaderTextured3D>();
}

void ShaderPrepass::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	auto sampleCount = GetSampleCount(pipelineIdx);
	prosper::util::RenderPassCreateInfo rpInfo {{{
		RENDER_PASS_NORMAL_FORMAT,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::DONT_CARE,
		Anvil::AttachmentStoreOp::STORE,sampleCount,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL
	},get_depth_render_pass_attachment_info(sampleCount)}};
	CreateCachedRenderPass<ShaderPrepass>({rpInfo},outRenderPass,pipelineIdx);
}

void ShaderPrepass::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderPrepassBase::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_NORMAL);
}

 // prosper TODO
#if 0
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include <pragma/model/vertex.h>

extern DLLCLIENT CGame *c_game;

using namespace Shader;

LINK_SHADER_TO_CLASS(PrepassDepth,prepass_depth);

PrepassBase::PrepassBase(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: Base3D(identifier,vsShader,fsShader,gsShader)
{
	SetUseDepth(true);
}
PrepassBase::PrepassBase()
	: PrepassBase("prepass_depth","world/prepass/vs_prepass_depth","")
{}

void PrepassBase::BindMaterial(Material *mat) {}

void PrepassBase::InitializeShaderPipelines(const Vulkan::Context &context)
{
	Base3D::InitializeShaderPipelines(context);

	m_reflectionPipelineIdx = InitializeShaderPipeline(context);
	m_renderPasses.push_back(m_renderPasses.front());
}

void PrepassBase::SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info)
{
	Base3D::SetupPipeline(pipelineIdx,info);

	auto &depthStencilState = const_cast<vk::PipelineDepthStencilStateCreateInfo&>(*info.pDepthStencilState);
	depthStencilState.setDepthCompareOp(vk::CompareOp::eLess);
	depthStencilState.setDepthWriteEnable(true);

	if(pipelineIdx == m_reflectionPipelineIdx)
		const_cast<vk::PipelineRasterizationStateCreateInfo*>(info.pRasterizationState)->cullMode = Anvil::CullModeFlagBits::CULL_MODE_FRONT_BIT;
}

void PrepassBase::InitializeRenderPasses()
{
	auto sampleCount = static_cast<Anvil::SampleCountFlagBits>(c_game->GetMSAASampleCount());
	SetSampleCount(sampleCount);
	std::vector<Vulkan::RenderPass::Attachment> attachments = {
		{Anvil::Format::D32_SFLOAT,sampleCount,true} // Depth Attachment
	};
	m_renderPasses = {m_context->GenerateRenderPass(attachments)};
}

bool PrepassBase::BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline)
{
	if(Base3D::BeginDraw(cmdBuffer,shaderPipeline) == false)
		return false;
	auto &context = *m_context.get();
	auto &drawCmd = context.GetDrawCmd();
	auto &layout = shaderPipeline->GetPipelineLayout();

	drawCmd->PushConstants(layout,Anvil::ShaderStageFlagBits::VERTEX_BIT,4,&c_game->GetRenderClipPlane());
	return true;
}

bool PrepassBase::BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,PipelineType type)
{
	switch(type)
	{
		case PipelineType::Regular:
		{
			return Base3D::BeginDraw(cmdBuffer);
		}
		case PipelineType::Reflection:
		{
			if(m_reflectionPipelineIdx >= m_pipelines.size())
				return false;
			auto &pipeline = m_pipelines.at(m_reflectionPipelineIdx);
			auto r = BeginDraw(cmdBuffer,pipeline.get());
			if(r == true)
				SetActivePipeline(m_reflectionPipelineIdx);
			return r;
		}
		default:
			return false;
	}
}

void PrepassBase::EndDraw(Vulkan::CommandBufferObject *cmdBuffer)
{
	SetActivePipeline(0u);
	Base3D::EndDraw(cmdBuffer);
}

void PrepassBase::InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions)
{
	Base::InitializeVertexDescriptions(vertexBindingDescriptions,vertexAttributeDescriptions);
	vertexBindingDescriptions.push_back({
		umath::to_integral(Binding::Vertex),
		sizeof(Vertex),
		Anvil::VertexInputRate::VERTEX
	});
	vertexBindingDescriptions.push_back({
		umath::to_integral(Binding::BoneWeight),
		sizeof(VertexWeight),
		Anvil::VertexInputRate::VERTEX
	});

	vertexAttributeDescriptions.push_back({
		umath::to_integral(Location::Vertex),
		umath::to_integral(Binding::Vertex),
		Anvil::Format::R32G32B32_SFLOAT,0 // TODO: Use 4 Byte floats?
	});
	vertexAttributeDescriptions.push_back({
		umath::to_integral(Location::BoneWeightId),
		umath::to_integral(Binding::BoneWeightId),
		vk::Format::eR32G32B32A32Sint,0
	});
	vertexAttributeDescriptions.push_back({
		umath::to_integral(Location::BoneWeight),
		umath::to_integral(Binding::BoneWeight),
		Anvil::Format::R32G32B32A32_SFLOAT,sizeof(Vector4i)
	});
}

void PrepassBase::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	Base3D::InitializePipelineLayout(context,setLayouts,pushConstants);

	pushConstants.push_back({
		Anvil::ShaderStageFlagBits::VERTEX_BIT,0,5
	});

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::UNIFORM_BUFFER_DYNAMIC,Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT}, // Instance
		{Anvil::DescriptorType::UNIFORM_BUFFER_DYNAMIC,Anvil::ShaderStageFlagBits::VERTEX_BIT}, // Bone Matrices
		{Anvil::DescriptorType::STORAGE_BUFFER,Anvil::ShaderStageFlagBits::VERTEX_BIT}, // Vertex Animations
		{Anvil::DescriptorType::STORAGE_BUFFER,Anvil::ShaderStageFlagBits::VERTEX_BIT} // Vertex Animation Frame Data
	}));
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT}, // Camera
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT}, // Render Settings
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // Dummy
	}));
}

void PrepassBase::BindScene(Vulkan::CommandBufferObject *cmdBuffer,const Scene &scene,bool bView)
{
	auto &context = *m_context.get();
	auto &drawCmd = context.GetDrawCmd();
	auto &layout = GetPipeline()->GetPipelineLayout();
	auto descSet = (bView == true) ? scene.GetViewCameraDescriptorSet() : scene.GetCameraDescriptorSetGraphics();
	drawCmd->BindDescriptorSet(umath::to_integral(DescSet::Camera),layout,descSet);
}

void PrepassBase::PushVertexAnimationOffset(Vulkan::CommandBufferObject *cmdBuffer,uint32_t offset)
{
	//if(offset == m_vertexAnimOffset)
	//	return;
	m_vertexAnimOffset = offset;
	cmdBuffer->PushConstants(GetPipeline()->GetPipelineLayout(),Anvil::ShaderStageFlagBits::VERTEX_BIT,4u,1u,&offset);
}

void PrepassBase::SetVertexAnimationOffset(Vulkan::CommandBufferObject *cmdBuffer,uint32_t offset) {PushVertexAnimationOffset(cmdBuffer,offset);}
void PrepassBase::SetVertexAnimationOffset(Vulkan::CommandBufferObject *cmdBuffer) {PushVertexAnimationOffset(cmdBuffer,0u);}

///////////////////////////

LINK_SHADER_TO_CLASS(Prepass,prepass);

Prepass::Prepass()
	: PrepassBase("prepass","world/prepass/vs_prepass","world/prepass/fs_prepass")
{}
void Prepass::InitializeAttachments(std::vector<vk::PipelineColorBlendAttachmentState> &attachments)
{
	PrepassBase::InitializeAttachments(attachments);
	attachments.push_back(attachments.front()); // Normal attachment
}

void Prepass::InitializeRenderPasses()
{
	auto sampleCount = static_cast<Anvil::SampleCountFlagBits>(c_game->GetMSAASampleCount());
	SetSampleCount(sampleCount);
	std::vector<Vulkan::RenderPass::Attachment> attachments = {
		{Anvil::Format::R16G16B16A16_SFLOAT,Anvil::SampleCountFlagBits::_1_BIT,false}, // Normal Attachment
		{Anvil::Format::D32_SFLOAT,sampleCount,true} // Depth Attachment
	};
	m_renderPasses = {m_context->GenerateRenderPass(attachments)};
}

void Prepass::InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions)
{
	PrepassBase::InitializeVertexDescriptions(vertexBindingDescriptions,vertexAttributeDescriptions);
	vertexAttributeDescriptions.push_back({
		umath::to_integral(Location::Normal),
		umath::to_integral(Binding::Normal),
		Anvil::Format::R32G32B32_SFLOAT,sizeof(Vector3) +sizeof(Vector2)
	});
}

#endif