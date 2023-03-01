/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_test.hpp"
#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"
#include "cmaterialmanager.h"
#include "pragma/entities/environment/c_env_reflection_probe.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/model/vk_mesh.h"
#include "pragma/model/c_modelmesh.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_command_buffer.hpp>
#include <texture_type.h>
#include <cmaterial_manager2.hpp>
#include <cmaterial.h>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

decltype(ShaderTest::DESCRIPTOR_SET_MATERIAL) ShaderTest::DESCRIPTOR_SET_MATERIAL = {{prosper::DescriptorSetInfo::Binding {// Material settings
                                                                                        prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::GeometryBit},
  prosper::DescriptorSetInfo::Binding {// Albedo Map
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Normal Map
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// RMA Map
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Emission Map
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Parallax Map
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Wrinkle Stretch Map
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Wrinkle Compress Map
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Exponent Map
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}}};
static_assert(umath::to_integral(ShaderTest::MaterialBinding::Count) == 9, "Number of bindings in material descriptor set does not match MaterialBinding enum count!");

decltype(ShaderTest::DESCRIPTOR_SET_PBR) ShaderTest::DESCRIPTOR_SET_PBR = {{prosper::DescriptorSetInfo::Binding {// Irradiance Map
                                                                              prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Prefilter Map
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// BRDF Map
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}}};
ShaderTest::ShaderTest(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader) : ShaderGameWorldLightingPass {context, identifier, vsShader, fsShader, gsShader}
{
	//SetPipelineCount(umath::to_integral(Pipeline::Count));
}
ShaderTest::ShaderTest(prosper::IPrContext &context, const std::string &identifier) : ShaderTest {context, identifier, "world/vs_test", "world/fs_test"} {}
void ShaderTest::DrawTest(prosper::IBuffer &buf, prosper::IBuffer &ibuf, uint32_t count)
{
	/*auto x = RecordBindIndexBuffer(ibuf) && RecordBindVertexBuffer(buf) &&
		ShaderScene::RecordPushConstants(sizeof(Mat4),&m_testMvp) &&
		RecordDrawIndexed(count);*/
}
prosper::DescriptorSetInfo &ShaderTest::GetMaterialDescriptorSetInfo() const { return ShaderGameWorldLightingPass::DESCRIPTOR_SET_MATERIAL; }
void ShaderTest::SetForceNonIBLMode(bool b) { m_bNonIBLMode = b; }
#if 0
bool ShaderTest::BeginDraw(
	const std::shared_ptr<prosper::ICommandBuffer> &cmdBuffer,const Vector4 &clipPlane,
	const Vector4 &drawOrigin,RecordFlags recordFlags
)
{
	return ShaderGraphics::BeginDraw(cmdBuffer) == true &&
		cmdBuffer->RecordSetDepthBias(1.f,0.f,0.f);
}
#endif
void ShaderTest::InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { AttachPushConstantRange(pipelineInfo, pipelineIdx, 0u, sizeof(Mat4), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit); }
void ShaderTest::UpdateRenderFlags(CModelSubMesh &mesh, SceneFlags &inOutFlags)
{
	ShaderGameWorldLightingPass::UpdateRenderFlags(mesh, inOutFlags);
	inOutFlags |= m_extRenderFlags;
}
void ShaderTest::InitializeGfxPipelineDescriptorSets(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	//ShaderGameWorldLightingPass::InitializeGfxPipelineDescriptorSets(pipelineInfo,pipelineIdx);
	//AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_PBR);
}
void ShaderTest::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderScene::InitializeGfxPipeline(pipelineInfo, pipelineIdx);
	//ShaderGameWorldLightingPass::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	//ShaderEntity::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	//ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	//if(pipelineIdx == umath::to_integral(ShaderGameWorldPipeline::Reflection))
	//	prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo,prosper::CullModeFlags::FrontBit);

	//pipelineInfo.ToggleDepthWrites(false);
	//pipelineInfo.ToggleDepthTest(true,prosper::CompareOp::LessOrEqual);

	//pipelineInfo.ToggleDepthBias(true,0.f,0.f,0.f);
	//pipelineInfo.ToggleDynamicState(true,prosper::DynamicState::DepthBias); // Required for decals

	//SetGenericAlphaColorBlendAttachmentProperties(pipelineInfo);
	//AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX);

	//AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT_ID);
	//AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT);

	//AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID);
	//AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT);

	AddVertexAttribute(pipelineInfo, VERTEX_ATTRIBUTE_POSITION);
	/*AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_UV);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_NORMAL);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_TANGENT);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BI_TANGENT);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_LIGHTMAP_UV);*/
	InitializeGfxPipelinePushConstantRanges(pipelineInfo, pipelineIdx);

	prosper::util::set_generic_alpha_color_blend_attachment_properties(pipelineInfo);
	pipelineInfo.ToggleDepthBias(true, 1.f, 0.f, 0.f);
	pipelineInfo.ToggleDynamicStates(true, {prosper::DynamicState::DepthBias});
	//InitializeGfxPipelineDescriptorSets(pipelineInfo,pipelineIdx);

	//ToggleDynamicScissorState(pipelineInfo,true);
	/*ShaderScene::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	prosper::util::set_generic_alpha_color_blend_attachment_properties(pipelineInfo);
	pipelineInfo.ToggleDepthBias(true,1.f,0.f,0.f);
	pipelineInfo.ToggleDynamicStates(true,{prosper::DynamicState::DepthBias});

	VERTEX_BINDING_VERTEX.stride = std::numeric_limits<decltype(VERTEX_BINDING_VERTEX.stride)>::max();

	//AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT_ID);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_UV);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_NORMAL);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_TANGENT);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BI_TANGENT);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_LIGHTMAP_UV);


	AttachPushConstantRange(pipelineInfo,0u,sizeof(Mat4),prosper::ShaderStageFlags::VertexBit);*/
}

static bool bind_texture(Material &mat, prosper::IDescriptorSet &ds, TextureInfo *texInfo, uint32_t bindingIndex, Texture *optDefaultTex = nullptr)
{
	auto &matManager = static_cast<msys::CMaterialManager &>(client->GetMaterialManager());
	auto &texManager = matManager.GetTextureManager();

	std::shared_ptr<Texture> tex = nullptr;
	if(texInfo && texInfo->texture)
		tex = std::static_pointer_cast<Texture>(texInfo->texture);
	else if(optDefaultTex == nullptr)
		return false;
	else
		tex = optDefaultTex->shared_from_this();
	if(tex && tex->HasValidVkTexture())
		ds.SetBindingTexture(*tex->GetVkTexture(), bindingIndex);
	return true;
}

static bool bind_default_texture(prosper::IDescriptorSet &ds, const std::string &defaultTexName, uint32_t bindingIndex)
{
	auto &matManager = static_cast<msys::CMaterialManager &>(client->GetMaterialManager());
	auto &texManager = matManager.GetTextureManager();
	auto ptrTex = texManager.LoadAsset(defaultTexName);
	if(ptrTex == nullptr)
		return false;
	if(ptrTex && ptrTex->HasValidVkTexture())
		ds.SetBindingTexture(*ptrTex->GetVkTexture(), bindingIndex);
	return true;
}

static bool bind_texture(Material &mat, prosper::IDescriptorSet &ds, TextureInfo *texInfo, uint32_t bindingIndex, const std::string &defaultTexName)
{
	auto &matManager = static_cast<msys::CMaterialManager &>(client->GetMaterialManager());
	auto &texManager = matManager.GetTextureManager();

	std::shared_ptr<Texture> tex = nullptr;
	if(texInfo && texInfo->texture) {
		tex = std::static_pointer_cast<Texture>(texInfo->texture);
		if(tex->HasValidVkTexture()) {
			ds.SetBindingTexture(*tex->GetVkTexture(), bindingIndex);
			return true;
		}
	}
	else if(defaultTexName.empty())
		return false;
	return bind_default_texture(ds, defaultTexName, bindingIndex);
}

std::shared_ptr<prosper::IDescriptorSetGroup> ShaderTest::InitializeMaterialDescriptorSet(CMaterial &mat, const prosper::DescriptorSetInfo &descSetInfo)
{
	auto *albedoMap = mat.GetDiffuseMap();
	if(albedoMap == nullptr || albedoMap->texture == nullptr)
		return nullptr;

	auto albedoTexture = std::static_pointer_cast<Texture>(albedoMap->texture);
	if(albedoTexture->HasValidVkTexture() == false)
		return nullptr;
	auto descSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(descSetInfo);
	mat.SetDescriptorSetGroup(*this, descSetGroup);
	auto &descSet = *descSetGroup->GetDescriptorSet();
	descSet.SetBindingTexture(*albedoTexture->GetVkTexture(), umath::to_integral(MaterialBinding::AlbedoMap));
	auto matData = InitializeMaterialBuffer(descSet, mat);
	if(matData.has_value() == false)
		return nullptr;

	if(bind_texture(mat, descSet, mat.GetNormalMap(), umath::to_integral(MaterialBinding::NormalMap), "black") == false)
		return nullptr;

	if(bind_texture(mat, descSet, mat.GetRMAMap(), umath::to_integral(MaterialBinding::RMAMap), "pbr/rma_neutral") == false)
		return nullptr;

	bind_texture(mat, descSet, mat.GetGlowMap(), umath::to_integral(MaterialBinding::EmissionMap));

	if(bind_texture(mat, descSet, mat.GetParallaxMap(), umath::to_integral(MaterialBinding::ParallaxMap), "black") == false)
		return nullptr;

	if(bind_texture(mat, descSet, mat.GetTextureInfo("wrinkle_stretch_map"), umath::to_integral(MaterialBinding::WrinkleStretchMap), albedoTexture.get()) == false)
		return nullptr;

	if(bind_texture(mat, descSet, mat.GetTextureInfo("wrinkle_compress_map"), umath::to_integral(MaterialBinding::WrinkleCompressMap), albedoTexture.get()) == false)
		return nullptr;

	if(bind_texture(mat, descSet, mat.GetTextureInfo("exponent_map"), umath::to_integral(MaterialBinding::ExponentMap), "white") == false)
		return nullptr;

	// TODO: FIXME: It would probably be a good idea to update the descriptor set lazily (i.e. not update it here), but
	// that seems to cause crashes in some cases
	if(descSet.Update() == false)
		return nullptr;
	return descSetGroup;
}
std::shared_ptr<prosper::IDescriptorSetGroup> ShaderTest::InitializeMaterialDescriptorSet(CMaterial &mat) { return InitializeMaterialDescriptorSet(mat, ShaderPBR::DESCRIPTOR_SET_MATERIAL); }
