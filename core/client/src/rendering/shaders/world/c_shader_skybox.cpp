/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_skybox.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <pragma/model/vertex.h>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;


decltype(ShaderSkybox::VERTEX_BINDING_VERTEX) ShaderSkybox::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex,sizeof(VertexBufferData)};
decltype(ShaderSkybox::VERTEX_ATTRIBUTE_POSITION) ShaderSkybox::VERTEX_ATTRIBUTE_POSITION = {ShaderTextured3DBase::VERTEX_ATTRIBUTE_POSITION,VERTEX_BINDING_VERTEX};
decltype(ShaderSkybox::DESCRIPTOR_SET_INSTANCE) ShaderSkybox::DESCRIPTOR_SET_INSTANCE = {&ShaderEntity::DESCRIPTOR_SET_INSTANCE};
decltype(ShaderSkybox::DESCRIPTOR_SET_CAMERA) ShaderSkybox::DESCRIPTOR_SET_CAMERA = {&ShaderEntity::DESCRIPTOR_SET_CAMERA};
decltype(ShaderSkybox::DESCRIPTOR_SET_MATERIAL) ShaderSkybox::DESCRIPTOR_SET_MATERIAL = {
	{
		prosper::DescriptorSetInfo::Binding { // Skybox Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
ShaderSkybox::ShaderSkybox(prosper::IPrContext &context,const std::string &identifier)
	: ShaderSkybox(context,identifier,"world/vs_skybox","world/fs_skybox")
{}

ShaderSkybox::ShaderSkybox(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader)
	: ShaderTextured3DBase(context,identifier,vsShader,fsShader)
{
	// SetBaseShader<ShaderTextured3DBase>();
}

void ShaderSkybox::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	if(pipelineIdx == umath::to_integral(Pipeline::Reflection))
		prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo,prosper::CullModeFlags::FrontBit);

	pipelineInfo.ToggleDepthWrites(false);
	pipelineInfo.ToggleDepthTest(false,prosper::CompareOp::Always);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);

	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),prosper::ShaderStageFlags::FragmentBit);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_INSTANCE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CAMERA);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_MATERIAL);
	ToggleDynamicScissorState(pipelineInfo,true);
}

std::shared_ptr<prosper::IDescriptorSetGroup> ShaderSkybox::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	auto *skyboxMap = mat.GetTextureInfo("skybox");
	if(skyboxMap == nullptr || skyboxMap->texture == nullptr)
		return nullptr;
	auto skyboxTexture = std::static_pointer_cast<Texture>(skyboxMap->texture);
	if(skyboxTexture->HasValidVkTexture() == false)
		return nullptr;
	auto descSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(DESCRIPTOR_SET_MATERIAL);
	mat.SetDescriptorSetGroup(*this,descSetGroup);
	auto &descSet = *descSetGroup->GetDescriptorSet();
	descSet.SetBindingTexture(*skyboxTexture->GetVkTexture(),0u);
	return descSetGroup;
}
uint32_t ShaderSkybox::GetMaterialDescriptorSetIndex() const {return DESCRIPTOR_SET_MATERIAL.setIndex;}
bool ShaderSkybox::BeginDraw(
	const std::shared_ptr<prosper::IPrimaryCommandBuffer> &cmdBuffer,const Vector4 &drawOrigin,
	const Vector4 &clipPlane,Pipeline pipelineIdx,RecordFlags recordFlags
)
{
	return ShaderScene::BeginDraw(cmdBuffer,umath::to_integral(pipelineIdx),recordFlags);
}
bool ShaderSkybox::BindEntity(CBaseEntity &ent)
{
	if(ShaderTextured3DBase::BindEntity(ent) == false)
		return false;
	auto skyC = ent.GetComponent<CSkyboxComponent>();
	m_skyAngles = skyC.valid() ? skyC->GetSkyAngles() : EulerAngles{};
	return true;
}
bool ShaderSkybox::BindSceneCamera(const pragma::rendering::RasterizationRenderer &renderer,bool bView)
{
	auto &scene = renderer.GetScene();
	auto &cam = scene.GetActiveCamera();
	if(ShaderTextured3DBase::BindSceneCamera(renderer,bView) == false)
		return false;
	auto origin = cam.valid() ? cam->GetEntity().GetPosition() : uvec::ORIGIN;
	uvec::rotate(&origin,m_skyAngles);
	return RecordPushConstants(PushConstants{origin}) == true;
}
bool ShaderSkybox::BindMaterialParameters(CMaterial &mat) {return true;}
bool ShaderSkybox::BindRenderSettings(prosper::IDescriptorSet &descSetRenderSettings) {return true;}
bool ShaderSkybox::BindLights(prosper::IDescriptorSet &dsLights) {return true;}
bool ShaderSkybox::BindVertexAnimationOffset(uint32_t offset) {return true;}
bool ShaderSkybox::Draw(CModelSubMesh &mesh) {return ShaderTextured3DBase::Draw(mesh,false);}

//////////////

ShaderSkyboxEquirect::ShaderSkyboxEquirect(prosper::IPrContext &context,const std::string &identifier)
	: ShaderSkybox{context,identifier,"world/vs_skybox_equirect","world/fs_skybox_equirect"}
{}

