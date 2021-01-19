/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"
#include "cmaterialmanager.h"
#include "pragma/entities/environment/c_env_reflection_probe.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/model/vk_mesh.h"
#include "pragma/model/c_modelmesh.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <texture_type.h>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCENGINE CEngine *c_engine;

using namespace pragma;
#pragma optimize("",off)
decltype(ShaderPBR::DESCRIPTOR_SET_MATERIAL) ShaderPBR::DESCRIPTOR_SET_MATERIAL = {
	{
		prosper::DescriptorSetInfo::Binding { // Material settings
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::GeometryBit
		},
		prosper::DescriptorSetInfo::Binding { // Albedo Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Normal Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // RMA Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Emission Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Parallax Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Wrinkle Stretch Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Wrinkle Compress Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Exponent Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
static_assert(umath::to_integral(ShaderPBR::MaterialBinding::Count) == 9,"Number of bindings in material descriptor set does not match MaterialBinding enum count!");

decltype(ShaderPBR::DESCRIPTOR_SET_PBR) ShaderPBR::DESCRIPTOR_SET_PBR = {
	{
		prosper::DescriptorSetInfo::Binding { // Irradiance Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Prefilter Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // BRDF Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
ShaderPBR::ShaderPBR(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: ShaderGameWorldLightingPass{context,identifier,vsShader,fsShader,gsShader}
{}
ShaderPBR::ShaderPBR(prosper::IPrContext &context,const std::string &identifier)
	: ShaderPBR{context,identifier,"world/vs_textured","world/pbr/fs_pbr"}
{
}
bool ShaderPBR::BindMaterialParameters(CMaterial &mat)
{
	return ShaderGameWorldLightingPass::BindMaterialParameters(mat);
}
prosper::DescriptorSetInfo &ShaderPBR::GetMaterialDescriptorSetInfo() const {return DESCRIPTOR_SET_MATERIAL;}
void ShaderPBR::SetForceNonIBLMode(bool b) {m_bNonIBLMode = b;}
bool ShaderPBR::BeginDraw(
	const std::shared_ptr<prosper::ICommandBuffer> &cmdBuffer,const Vector4 &clipPlane,
	const Vector4 &drawOrigin,RecordFlags recordFlags
)
{
	m_extRenderFlags = SceneFlags::None;
	return ShaderGameWorldLightingPass::BeginDraw(cmdBuffer,clipPlane,drawOrigin,recordFlags);
}
bool ShaderPBR::BindSceneCamera(pragma::CSceneComponent &scene,const rendering::RasterizationRenderer &renderer,bool bView)
{
	if(ShaderGameWorldLightingPass::BindSceneCamera(scene,renderer,bView) == false)
		return false;
	auto hCam = scene.GetActiveCamera();
	if(hCam.expired())
		return false;
	if(m_bNonIBLMode == false)
	{
		float iblStrength;
		auto *ds = CReflectionProbeComponent::FindDescriptorSetForClosestProbe(scene,hCam->GetEntity().GetPosition(),iblStrength);
		if(ds)
			return BindReflectionProbeIntensity(iblStrength) && RecordBindDescriptorSet(*ds,DESCRIPTOR_SET_PBR.setIndex);
	}
	// No reflection probe and therefore no IBL available. Fallback to non-IBL rendering.
	m_extRenderFlags |= SceneFlags::NoIBL;
	return RecordBindDescriptorSet(*m_defaultPbrDsg->GetDescriptorSet(),DESCRIPTOR_SET_PBR.setIndex);
}
void ShaderPBR::UpdateRenderFlags(CModelSubMesh &mesh,SceneFlags &inOutFlags)
{
	ShaderGameWorldLightingPass::UpdateRenderFlags(mesh,inOutFlags);
	inOutFlags |= m_extRenderFlags;
}
void ShaderPBR::InitializeGfxPipelineDescriptorSets(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGameWorldLightingPass::InitializeGfxPipelineDescriptorSets(pipelineInfo,pipelineIdx);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_PBR);
}

static bool bind_texture(Material &mat,prosper::IDescriptorSet &ds,TextureInfo *texInfo,uint32_t bindingIndex,Texture *optDefaultTex=nullptr)
{
	auto &matManager = static_cast<CMaterialManager&>(client->GetMaterialManager());
	auto &texManager = matManager.GetTextureManager();

	TextureManager::LoadInfo loadInfo {};
	loadInfo.flags = TextureLoadFlags::LoadInstantly;
	loadInfo.mipmapLoadMode = TextureMipmapMode::Load;

	std::shared_ptr<Texture> tex = nullptr;
	if(texInfo && texInfo->texture)
		tex = std::static_pointer_cast<Texture>(texInfo->texture);
	else if(optDefaultTex == nullptr)
		return false;
	else
		tex = optDefaultTex->shared_from_this();
	if(tex && tex->HasValidVkTexture())
		ds.SetBindingTexture(*tex->GetVkTexture(),bindingIndex);
	return true;
}

static TextureManager::LoadInfo get_texture_load_info()
{
	TextureManager::LoadInfo loadInfo {};
	loadInfo.flags = TextureLoadFlags::LoadInstantly;
	loadInfo.mipmapLoadMode = TextureMipmapMode::Load;
	return loadInfo;
}

static bool bind_default_texture(prosper::IDescriptorSet &ds,const std::string &defaultTexName,uint32_t bindingIndex)
{
	auto &matManager = static_cast<CMaterialManager&>(client->GetMaterialManager());
	auto &texManager = matManager.GetTextureManager();
	std::shared_ptr<void> ptrTex = nullptr;
	if(texManager.Load(c_engine->GetRenderContext(),defaultTexName,get_texture_load_info(),&ptrTex) == false)
		return false;
	auto tex = std::static_pointer_cast<Texture>(ptrTex);

	if(tex && tex->HasValidVkTexture())
		ds.SetBindingTexture(*tex->GetVkTexture(),bindingIndex);
	return true;
}

static bool bind_texture(Material &mat,prosper::IDescriptorSet &ds,TextureInfo *texInfo,uint32_t bindingIndex,const std::string &defaultTexName)
{
	auto &matManager = static_cast<CMaterialManager&>(client->GetMaterialManager());
	auto &texManager = matManager.GetTextureManager();

	std::shared_ptr<Texture> tex = nullptr;
	if(texInfo && texInfo->texture)
	{
		tex = std::static_pointer_cast<Texture>(texInfo->texture);
		if(tex->HasValidVkTexture())
		{
			ds.SetBindingTexture(*tex->GetVkTexture(),bindingIndex);
			return true;
		}
	}
	else if(defaultTexName.empty())
		return false;
	return bind_default_texture(ds,defaultTexName,bindingIndex);
}

std::shared_ptr<prosper::IDescriptorSetGroup> ShaderPBR::InitializeMaterialDescriptorSet(CMaterial &mat,const prosper::DescriptorSetInfo &descSetInfo)
{
	auto *albedoMap = mat.GetDiffuseMap();
	if(albedoMap == nullptr || albedoMap->texture == nullptr)
		return nullptr;

	auto albedoTexture = std::static_pointer_cast<Texture>(albedoMap->texture);
	if(albedoTexture->HasValidVkTexture() == false)
		return nullptr;
	auto descSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(descSetInfo);
	mat.SetDescriptorSetGroup(*this,descSetGroup);
	auto &descSet = *descSetGroup->GetDescriptorSet();
	descSet.SetBindingTexture(*albedoTexture->GetVkTexture(),umath::to_integral(MaterialBinding::AlbedoMap));
	auto matData = InitializeMaterialBuffer(descSet,mat);
	if(matData.has_value() == false)
		return nullptr;

	if(bind_texture(mat,descSet,mat.GetNormalMap(),umath::to_integral(MaterialBinding::NormalMap),"black") == false)
		return false;

	if(bind_texture(mat,descSet,mat.GetRMAMap(),umath::to_integral(MaterialBinding::RMAMap),"pbr/rma_neutral") == false)
		return false;

	bind_texture(mat,descSet,mat.GetGlowMap(),umath::to_integral(MaterialBinding::EmissionMap));

	if(bind_texture(mat,descSet,mat.GetParallaxMap(),umath::to_integral(MaterialBinding::ParallaxMap),"black") == false)
		return false;

	if(bind_texture(mat,descSet,mat.GetTextureInfo("wrinkle_stretch_map"),umath::to_integral(MaterialBinding::WrinkleStretchMap),albedoTexture.get()) == false)
		return false;

	if(bind_texture(mat,descSet,mat.GetTextureInfo("wrinkle_compress_map"),umath::to_integral(MaterialBinding::WrinkleCompressMap),albedoTexture.get()) == false)
		return false;

	if(bind_texture(mat,descSet,mat.GetTextureInfo("exponent_map"),umath::to_integral(MaterialBinding::ExponentMap),"white") == false)
		return false;

	// TODO: FIXME: It would probably be a good idea to update the descriptor set lazily (i.e. not update it here), but
	// that seems to cause crashes in some cases
	if(descSet.Update() == false)
		return false;
	return descSetGroup;
}
void ShaderPBR::OnPipelinesInitialized()
{
	ShaderGameWorldLightingPass::OnPipelinesInitialized();
	m_defaultPbrDsg = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderPBR::DESCRIPTOR_SET_PBR);
	m_defaultMatDsg = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderPBR::DESCRIPTOR_SET_MATERIAL);
}
prosper::IDescriptorSet &ShaderPBR::GetDefaultPbrDescriptorSet() const {return *m_defaultPbrDsg->GetDescriptorSet();}
std::shared_ptr<prosper::IDescriptorSetGroup> ShaderPBR::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	return InitializeMaterialDescriptorSet(mat,DESCRIPTOR_SET_MATERIAL);
}
void ShaderPBR::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGameWorldLightingPass::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
}

/////////////////

decltype(ShaderPBRBlend::VERTEX_BINDING_ALPHA) ShaderPBRBlend::VERTEX_BINDING_ALPHA = {prosper::VertexInputRate::Vertex};
decltype(ShaderPBRBlend::VERTEX_ATTRIBUTE_ALPHA) ShaderPBRBlend::VERTEX_ATTRIBUTE_ALPHA = {VERTEX_BINDING_ALPHA,prosper::Format::R32G32_SFloat};
decltype(ShaderPBRBlend::DESCRIPTOR_SET_MATERIAL) ShaderPBRBlend::DESCRIPTOR_SET_MATERIAL = {
	&ShaderPBR::DESCRIPTOR_SET_MATERIAL,
	{
		prosper::DescriptorSetInfo::Binding { // Albedo Map 2
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Albedo Map 3
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
ShaderPBRBlend::ShaderPBRBlend(prosper::IPrContext &context,const std::string &identifier)
	: ShaderPBR{context,identifier,"world/vs_textured_blend","world/pbr/fs_pbr_blend"}
{}
void ShaderPBRBlend::InitializeGfxPipelineVertexAttributes(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderPBR::InitializeGfxPipelineVertexAttributes(pipelineInfo,pipelineIdx);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_ALPHA);
}
prosper::DescriptorSetInfo &ShaderPBRBlend::GetMaterialDescriptorSetInfo() const {return DESCRIPTOR_SET_MATERIAL;}
void ShaderPBRBlend::InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	AttachPushConstantRange(pipelineInfo,0u,sizeof(ShaderGameWorldLightingPass::PushConstants) +sizeof(PushConstants),prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit);
}
std::shared_ptr<prosper::IDescriptorSetGroup> ShaderPBRBlend::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	auto descSetGroup = ShaderPBR::InitializeMaterialDescriptorSet(mat,DESCRIPTOR_SET_MATERIAL);
	if(descSetGroup == nullptr)
		return nullptr;
	auto &descSet = *descSetGroup->GetDescriptorSet();

	auto *diffuseMap2 = mat.GetTextureInfo(Material::ALBEDO_MAP2_IDENTIFIER);
	if(diffuseMap2 != nullptr && diffuseMap2->texture != nullptr)
	{
		auto texture = std::static_pointer_cast<Texture>(diffuseMap2->texture);
		if(texture->HasValidVkTexture())
			descSet.SetBindingTexture(*texture->GetVkTexture(),umath::to_integral(MaterialBinding::AlbedoMap2));
	}

	auto *diffuseMap3 = mat.GetTextureInfo(Material::ALBEDO_MAP3_IDENTIFIER);
	if(diffuseMap3 != nullptr && diffuseMap3->texture != nullptr)
	{
		auto texture = std::static_pointer_cast<Texture>(diffuseMap3->texture);
		if(texture->HasValidVkTexture())
			descSet.SetBindingTexture(*texture->GetVkTexture(),umath::to_integral(MaterialBinding::AlbedoMap3));
	}
	return descSetGroup;
}
bool ShaderPBRBlend::Draw(CModelSubMesh &mesh,const std::optional<pragma::RenderMeshIndex> &meshIdx,prosper::IBuffer &renderBufferIndexBuffer,uint32_t instanceCount)
{
	auto numAlpha = 0;
	auto alphaBuffer = c_engine->GetRenderContext().GetDummyBuffer();
	auto &vkMesh = mesh.GetSceneMesh();
	if(vkMesh != nullptr)
	{
		auto &meshAlphaBuffer = vkMesh->GetAlphaBuffer();
		if(meshAlphaBuffer != nullptr)
		{
			alphaBuffer = meshAlphaBuffer;
			numAlpha = mesh.GetAlphaCount();
		}
	}
	return RecordPushConstants(PushConstants{numAlpha},sizeof(ShaderPBR::PushConstants)) == true &&
		RecordBindVertexBuffer(*alphaBuffer,VERTEX_BINDING_VERTEX.GetBindingIndex() +2u) == true &&
		ShaderPBR::Draw(mesh,meshIdx,renderBufferIndexBuffer,instanceCount) == true;
}
bool ShaderPBRBlend::GetRenderBufferTargets(
	CModelSubMesh &mesh,uint32_t pipelineIdx,std::vector<prosper::IBuffer*> &outBuffers,std::vector<prosper::DeviceSize> &outOffsets,
	std::optional<prosper::IndexBufferInfo> &outIndexBufferInfo
) const
{
	if(ShaderPBR::GetRenderBufferTargets(mesh,pipelineIdx,outBuffers,outOffsets,outIndexBufferInfo) == false)
		return false;
	auto &sceneMesh = mesh.GetSceneMesh();
	auto *alphaBuf = sceneMesh->GetAlphaBuffer().get();
	outBuffers.push_back(alphaBuf);
	outOffsets.push_back(0ull);
	return true;
}
#pragma optimize("",on)
