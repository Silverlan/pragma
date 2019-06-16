#include "stdafx_client.h"
#include "pragma/rendering/shaders/c_shader_shadow.hpp"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include <prosper_util.hpp>
#include <pragma/model/vertex.h>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;

static auto SHADOW_DEPTH_BIAS_CONSTANT = 1.25f;
static auto SHADOW_DEPTH_BIAS_SLOPE = 1.75f;

decltype(ShaderShadow::RENDER_PASS_DEPTH_FORMAT) ShaderShadow::RENDER_PASS_DEPTH_FORMAT = Anvil::Format::D32_SFLOAT;
decltype(ShaderShadow::VERTEX_BINDING_BONE_WEIGHT) ShaderShadow::VERTEX_BINDING_BONE_WEIGHT = {Anvil::VertexInputRate::VERTEX};
decltype(ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID) ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID,VERTEX_BINDING_BONE_WEIGHT};
decltype(ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT) ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT,VERTEX_BINDING_BONE_WEIGHT};

decltype(ShaderShadow::VERTEX_BINDING_VERTEX) ShaderShadow::VERTEX_BINDING_VERTEX = {Anvil::VertexInputRate::VERTEX,sizeof(VertexBufferData)};
decltype(ShaderShadow::VERTEX_ATTRIBUTE_POSITION) ShaderShadow::VERTEX_ATTRIBUTE_POSITION = {ShaderEntity::VERTEX_ATTRIBUTE_POSITION,VERTEX_BINDING_VERTEX};

decltype(ShaderShadow::DESCRIPTOR_SET_INSTANCE) ShaderShadow::DESCRIPTOR_SET_INSTANCE = {&ShaderEntity::DESCRIPTOR_SET_INSTANCE};
ShaderShadow::ShaderShadow(prosper::Context &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader)
	: ShaderEntity(context,identifier,vsShader,fsShader)
{}

ShaderShadow::ShaderShadow(prosper::Context &context,const std::string &identifier)
	: ShaderShadow(context,identifier,"shadow/vs_shadow","shadow/fs_shadow")
{}
bool ShaderShadow::BindDepthMatrix(const Mat4 &depthMVP)
{
	return RecordPushConstants(depthMVP);
}
bool ShaderShadow::BindMaterial(CMaterial &mat)
{
	/*auto &descTexture = mat.GetDescriptorSetGroup(*this);
	if(descTexture == nullptr)
		return false;
	auto *data = mat.GetDataBlock();
	auto alphaDiscardThreshold = pragma::DefaultAlphaDiscardThreshold;
	if(data != nullptr)
		data->GetFloat("alpha_discard_threshold",&alphaDiscardThreshold);
	cmdBuffer->PushConstants(layout,Anvil::ShaderStageFlagBits::FRAGMENT_BIT,20,1,&alphaDiscardThreshold);
	cmdBuffer->BindDescriptorSet(umath::to_integral(DescSet::DiffuseMap),layout,descTexture);
	return true;*/
	return false; // prosper TODO
}

bool ShaderShadow::BindEntity(CBaseEntity &ent,const Mat4 &depthMVP)
{
	if(ShaderEntity::BindEntity(ent) == false)
		return false;
	auto pRenderComponent = ent.GetRenderComponent();
	if(pRenderComponent.expired())
		return false;
	auto entMvp = depthMVP *pRenderComponent->GetTransformationMatrix();
	return BindDepthMatrix(entMvp);
}
bool ShaderShadow::BindLight(CLightComponent &light)
{
	auto &ent = light.GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	auto pRadiusComponent = ent.GetComponent<CRadiusComponent>();
	auto pos = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
	auto lightPos = Vector4{pos.x,pos.y,pos.z,static_cast<float>(pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 0.f)};
	return RecordPushConstants(lightPos,offsetof(PushConstants,lightPos));
}
void ShaderShadow::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderShadow>({{{
		{
			RENDER_PASS_DEPTH_FORMAT,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::CLEAR,
			Anvil::AttachmentStoreOp::STORE,Anvil::SampleCountFlagBits::_1_BIT,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL
		}
	}}},outRenderPass,pipelineIdx);
}
uint32_t ShaderShadow::GetRenderSettingsDescriptorSetIndex() const {return std::numeric_limits<uint32_t>::max();}
uint32_t ShaderShadow::GetCameraDescriptorSetIndex() const {return std::numeric_limits<uint32_t>::max();}
uint32_t ShaderShadow::GetLightDescriptorSetIndex() const {return std::numeric_limits<uint32_t>::max();}
uint32_t ShaderShadow::GetInstanceDescriptorSetIndex() const{return DESCRIPTOR_SET_INSTANCE.setIndex;}
void ShaderShadow::GetVertexAnimationPushConstantInfo(uint32_t &offset) const {}
void ShaderShadow::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	ToggleDynamicScissorState(pipelineInfo,true);
	pipelineInfo.toggle_depth_writes(true);
	pipelineInfo.toggle_depth_test(true,Anvil::CompareOp::LESS_OR_EQUAL);
	prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo,Anvil::CullModeFlagBits::NONE);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT_ID);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);

	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),Anvil::ShaderStageFlagBits::VERTEX_BIT | Anvil::ShaderStageFlagBits::FRAGMENT_BIT);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_INSTANCE);

	pipelineInfo.toggle_depth_bias(true,SHADOW_DEPTH_BIAS_CONSTANT,0.f,SHADOW_DEPTH_BIAS_SLOPE);
}

//////////////////

ShaderShadowSpot::ShaderShadowSpot(prosper::Context &context,const std::string &identifier)
	: ShaderShadow(context,identifier,"shadow/vs_shadow","shadow/fs_shadow_spot")
{
	SetBaseShader<ShaderShadow>();
}

//////////////////

ShaderShadowCSM::ShaderShadowCSM(prosper::Context &context,const std::string &identifier)
	: ShaderShadow(context,identifier,"shadow/vs_shadow_csm","")
{
	SetBaseShader<ShaderShadow>();
}
void ShaderShadowCSM::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderShadowCSM>({{{
		{
			RENDER_PASS_DEPTH_FORMAT,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::LOAD,
			Anvil::AttachmentStoreOp::STORE,Anvil::SampleCountFlagBits::_1_BIT,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL
		}
	}}},outRenderPass,pipelineIdx);
}
