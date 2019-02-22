#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_skybox.hpp"
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <pragma/model/vertex.h>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;

decltype(ShaderSkybox::VERTEX_BINDING_VERTEX) ShaderSkybox::VERTEX_BINDING_VERTEX = {Anvil::VertexInputRate::VERTEX,sizeof(Vertex)};
decltype(ShaderSkybox::VERTEX_ATTRIBUTE_POSITION) ShaderSkybox::VERTEX_ATTRIBUTE_POSITION = {ShaderTextured3DBase::VERTEX_ATTRIBUTE_POSITION,VERTEX_BINDING_VERTEX};
decltype(ShaderSkybox::DESCRIPTOR_SET_INSTANCE) ShaderSkybox::DESCRIPTOR_SET_INSTANCE = {&ShaderEntity::DESCRIPTOR_SET_INSTANCE};
decltype(ShaderSkybox::DESCRIPTOR_SET_CAMERA) ShaderSkybox::DESCRIPTOR_SET_CAMERA = {&ShaderEntity::DESCRIPTOR_SET_CAMERA};
decltype(ShaderSkybox::DESCRIPTOR_SET_MATERIAL) ShaderSkybox::DESCRIPTOR_SET_MATERIAL = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Skybox Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
ShaderSkybox::ShaderSkybox(prosper::Context &context,const std::string &identifier)
	: ShaderTextured3DBase(context,identifier,"world/vs_skybox","world/fs_skybox")
{
	SetBaseShader<ShaderTextured3D>();
}

void ShaderSkybox::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	if(pipelineIdx == umath::to_integral(Pipeline::Reflection))
		prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo,Anvil::CullModeFlagBits::FRONT_BIT);

	pipelineInfo.toggle_depth_writes(false);
	pipelineInfo.toggle_depth_test(false,Anvil::CompareOp::ALWAYS);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);

	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),Anvil::ShaderStageFlagBits::FRAGMENT_BIT);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_INSTANCE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CAMERA);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_MATERIAL);
	ToggleDynamicScissorState(pipelineInfo,true);
}

std::shared_ptr<prosper::DescriptorSetGroup> ShaderSkybox::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	auto &dev = c_engine->GetDevice();
	auto *skyboxMap = mat.GetTextureInfo("skybox");
	if(skyboxMap == nullptr || skyboxMap->texture == nullptr)
		return nullptr;
	auto skyboxTexture = std::static_pointer_cast<Texture>(skyboxMap->texture);
	if(skyboxTexture->texture == nullptr)
		return nullptr;
	auto descSetGroup = prosper::util::create_descriptor_set_group(dev,DESCRIPTOR_SET_MATERIAL);
	mat.SetDescriptorSetGroup(*this,descSetGroup);
	auto descSet = (*descSetGroup)->get_descriptor_set(0u);
	prosper::util::set_descriptor_set_binding_texture(*descSet,*skyboxTexture->texture,0u);
	return descSetGroup;
}
uint32_t ShaderSkybox::GetMaterialDescriptorSetIndex() const {return DESCRIPTOR_SET_MATERIAL.setIndex;}
bool ShaderSkybox::BeginDraw(const std::shared_ptr<prosper::PrimaryCommandBuffer> &cmdBuffer,const Vector4 &clipPlane,Pipeline pipelineIdx,RecordFlags recordFlags)
{
	return ShaderScene::BeginDraw(cmdBuffer,umath::to_integral(pipelineIdx),recordFlags);
}
bool ShaderSkybox::BindSceneCamera(const Scene &scene,bool bView)
{
	return ShaderTextured3DBase::BindSceneCamera(scene,bView) == true &&
		RecordPushConstants(PushConstants{scene.GetCamera()->GetPos()}) == true;
}
bool ShaderSkybox::BindMaterialParameters(CMaterial &mat) {return true;}
bool ShaderSkybox::BindRenderSettings(Anvil::DescriptorSet &descSetRenderSettings) {return true;}
bool ShaderSkybox::BindLights(Anvil::DescriptorSet &descSetShadowMaps,Anvil::DescriptorSet &descSetLightSources) {return true;}
bool ShaderSkybox::BindVertexAnimationOffset(uint32_t offset) {return true;}
bool ShaderSkybox::Draw(CModelSubMesh &mesh) {return ShaderTextured3DBase::Draw(mesh,false);}
