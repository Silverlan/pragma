#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/model/c_vertex_buffer_data.hpp"
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

decltype(ShaderPrepassBase::VERTEX_BINDING_VERTEX) ShaderPrepassBase::VERTEX_BINDING_VERTEX = {Anvil::VertexInputRate::VERTEX,sizeof(VertexBufferData)};
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
