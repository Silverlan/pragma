/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_skybox.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_command_buffer.hpp>
#include <pragma/model/vertex.h>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;


decltype(ShaderSkybox::VERTEX_BINDING_VERTEX) ShaderSkybox::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex,sizeof(VertexBufferData)};
decltype(ShaderSkybox::VERTEX_ATTRIBUTE_POSITION) ShaderSkybox::VERTEX_ATTRIBUTE_POSITION = {ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_POSITION,VERTEX_BINDING_VERTEX};
decltype(ShaderSkybox::DESCRIPTOR_SET_INSTANCE) ShaderSkybox::DESCRIPTOR_SET_INSTANCE = {&ShaderEntity::DESCRIPTOR_SET_INSTANCE};
decltype(ShaderSkybox::DESCRIPTOR_SET_SCENE) ShaderSkybox::DESCRIPTOR_SET_SCENE = {&ShaderEntity::DESCRIPTOR_SET_SCENE};
decltype(ShaderSkybox::DESCRIPTOR_SET_RENDERER) ShaderSkybox::DESCRIPTOR_SET_RENDERER = {&ShaderEntity::DESCRIPTOR_SET_RENDERER};
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
	: ShaderGameWorldLightingPass(context,identifier,vsShader,fsShader)
{
	// SetBaseShader<ShaderTextured3DBase>();
}

prosper::DescriptorSetInfo &ShaderSkybox::GetMaterialDescriptorSetInfo() const {return DESCRIPTOR_SET_MATERIAL;}

bool ShaderSkybox::GetRenderBufferTargets(
	CModelSubMesh &mesh,uint32_t pipelineIdx,std::vector<prosper::IBuffer*> &outBuffers,std::vector<prosper::DeviceSize> &outOffsets,
	std::optional<prosper::IndexBufferInfo> &outIndexBufferInfo
) const
{
	auto &sceneMesh = mesh.GetSceneMesh();
	if(sceneMesh == nullptr)
		return false;
	auto &indexBuffer = sceneMesh->GetIndexBuffer();
	if(indexBuffer)
	{
		outIndexBufferInfo = prosper::IndexBufferInfo{};
		outIndexBufferInfo->buffer = indexBuffer;
		outIndexBufferInfo->indexType = prosper::IndexType::UInt16;
		outIndexBufferInfo->offset = 0;
	}

	auto *vBuf = sceneMesh->GetVertexBuffer().get();
	outBuffers = {vBuf};
	outOffsets = {0ull};
	return true;
}

void ShaderSkybox::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	//if(pipelineIdx == umath::to_integral(ShaderGameWorldPipeline::Reflection))
	//	prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo,prosper::CullModeFlags::FrontBit);

	pipelineInfo.ToggleDepthWrites(false);
	pipelineInfo.ToggleDepthTest(false,prosper::CompareOp::Always);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_INSTANCE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_MATERIAL);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_SCENE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_RENDERER);
	ToggleDynamicScissorState(pipelineInfo,true);
}

uint32_t ShaderSkybox::GetRenderSettingsDescriptorSetIndex() const {return std::numeric_limits<uint32_t>::max();}
uint32_t ShaderSkybox::GetRendererDescriptorSetIndex() const {return DESCRIPTOR_SET_RENDERER.setIndex;}
uint32_t ShaderSkybox::GetCameraDescriptorSetIndex() const {return std::numeric_limits<uint32_t>::max();}
uint32_t ShaderSkybox::GetLightDescriptorSetIndex() const {return std::numeric_limits<uint32_t>::max();}
uint32_t ShaderSkybox::GetInstanceDescriptorSetIndex() const{return DESCRIPTOR_SET_INSTANCE.setIndex;}
uint32_t ShaderSkybox::GetMaterialDescriptorSetIndex() const {return DESCRIPTOR_SET_MATERIAL.setIndex;}

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
	descSet.Update();
	return descSetGroup;
}
bool ShaderSkybox::BeginDraw(
	const std::shared_ptr<prosper::ICommandBuffer> &cmdBuffer,const Vector4 &drawOrigin,
	const Vector4 &clipPlane,RecordFlags recordFlags
)
{
	return ShaderScene::BeginDraw(cmdBuffer,0u,recordFlags);
}
bool ShaderSkybox::BindEntity(CBaseEntity &ent)
{
	if(ShaderGameWorldLightingPass::BindEntity(ent) == false)
		return false;
	auto skyC = ent.GetComponent<CSkyboxComponent>();
	m_skyAngles = skyC.valid() ? skyC->GetSkyAngles() : EulerAngles{};
	return true;
}
bool ShaderSkybox::BindSceneCamera(pragma::CSceneComponent &scene,const pragma::CRasterizationRendererComponent &renderer,bool bView)
{
	return ShaderGameWorldLightingPass::BindSceneCamera(scene,renderer,bView);
	/*auto &cam = scene.GetActiveCamera();
	if(ShaderGameWorldLightingPass::BindSceneCamera(scene,renderer,bView) == false)
		return false;
	auto origin = cam.valid() ? cam->GetEntity().GetPosition() : uvec::ORIGIN;
	uvec::rotate(&origin,m_skyAngles);
	return RecordPushConstants(PushConstants{origin}) == true;*/
}
bool ShaderSkybox::BindMaterialParameters(CMaterial &mat) {return true;}
bool ShaderSkybox::BindRenderSettings(prosper::IDescriptorSet &descSetRenderSettings) {return true;}
bool ShaderSkybox::BindLights(prosper::IDescriptorSet &dsLights) {return true;}
bool ShaderSkybox::BindVertexAnimationOffset(uint32_t offset) {return true;}
bool ShaderSkybox::Draw(CModelSubMesh &mesh,const std::optional<pragma::RenderMeshIndex> &meshIdx,prosper::IBuffer &renderBufferIndexBuffer,uint32_t instanceCount) {return ShaderGameWorldLightingPass::Draw(mesh,meshIdx,renderBufferIndexBuffer,false,instanceCount);}

//

void ShaderSkybox::RecordBindScene(
	rendering::ShaderProcessor &shaderProcessor,
	const pragma::CSceneComponent &scene,const pragma::CRasterizationRendererComponent &renderer,
	prosper::IDescriptorSet &dsScene,prosper::IDescriptorSet &dsRenderer,
	prosper::IDescriptorSet &dsRenderSettings,prosper::IDescriptorSet &dsLights,
	prosper::IDescriptorSet &dsShadows,prosper::IDescriptorSet &dsMaterial,
	ShaderGameWorld::SceneFlags &inOutSceneFlags
) const
{
	std::array<prosper::IDescriptorSet*,3> descSets {
		&dsMaterial,
		&dsScene,
		&dsRenderer
	};

	static const std::vector<uint32_t> dynamicOffsets {};
	shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics,shaderProcessor.GetCurrentPipelineLayout(),pragma::ShaderGameWorld::MATERIAL_DESCRIPTOR_SET_INDEX,descSets,dynamicOffsets);
}

//////////////

ShaderSkyboxEquirect::ShaderSkyboxEquirect(prosper::IPrContext &context,const std::string &identifier)
	: ShaderSkybox{context,identifier,"world/vs_skybox_equirect","world/fs_skybox_equirect"}
{}

