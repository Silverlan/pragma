/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"
#include "cmaterialmanager.h"
#include "pragma/entities/environment/c_env_reflection_probe.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/model/vk_mesh.h"
#include "pragma/model/c_modelmesh.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_command_buffer.hpp>
#include <cmaterial_manager2.hpp>
#include <texture_type.h>
#include <cmaterial.h>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

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

prosper::DescriptorSetInfo &ShaderPBR::GetMaterialDescriptorSetInfo() const {return DESCRIPTOR_SET_MATERIAL;}
void ShaderPBR::UpdateRenderFlags(CModelSubMesh &mesh,SceneFlags &inOutFlags)
{
	ShaderGameWorldLightingPass::UpdateRenderFlags(mesh,inOutFlags);
	inOutFlags |= m_extRenderFlags;
}
void ShaderPBR::InitializeGfxPipelineDescriptorSets(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGameWorldLightingPass::InitializeGfxPipelineDescriptorSets(pipelineInfo,pipelineIdx);
	AddDescriptorSetGroup(pipelineInfo,pipelineIdx,DESCRIPTOR_SET_PBR);
}

bool ShaderPBR::BindDescriptorSetTexture(Material &mat,prosper::IDescriptorSet &ds,TextureInfo *texInfo,uint32_t bindingIndex,Texture *optDefaultTex)
{
	auto &matManager = static_cast<msys::CMaterialManager&>(client->GetMaterialManager());
	auto &texManager = matManager.GetTextureManager();

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

static bool bind_default_texture(prosper::IDescriptorSet &ds,const std::string &defaultTexName,uint32_t bindingIndex,Texture **optOutTex)
{
	auto &matManager = static_cast<msys::CMaterialManager&>(client->GetMaterialManager());
	auto &texManager = matManager.GetTextureManager();
	auto *asset = texManager.FindCachedAsset(defaultTexName);
	if(!asset)
	{
		Con::cwar<<"WARNING: Attempted to bind texture '"<<defaultTexName<<"' to material descriptor set, but texture has not been loaded!"<<Con::endl;
		return false;
	}
	auto ptrTex = msys::TextureManager::GetAssetObject(*asset);
	if(ptrTex == nullptr)
		return false;
	auto tex = std::static_pointer_cast<Texture>(ptrTex);

	if(tex && tex->HasValidVkTexture())
		ds.SetBindingTexture(*tex->GetVkTexture(),bindingIndex);
	if(optOutTex)
		*optOutTex = tex.get();
	return true;
}

bool ShaderPBR::BindDescriptorSetTexture(
	Material &mat,prosper::IDescriptorSet &ds,TextureInfo *texInfo,uint32_t bindingIndex,const std::string &defaultTexName,Texture **optOutTex
)
{
	auto &matManager = static_cast<msys::CMaterialManager&>(client->GetMaterialManager());
	auto &texManager = matManager.GetTextureManager();

	std::shared_ptr<Texture> tex = nullptr;
	if(texInfo && texInfo->texture)
	{
		tex = std::static_pointer_cast<Texture>(texInfo->texture);
		if(tex->HasValidVkTexture())
		{
			ds.SetBindingTexture(*tex->GetVkTexture(),bindingIndex);
			if(optOutTex)
				*optOutTex = tex.get();
			return true;
		}
	}
	else if(defaultTexName.empty())
		return false;
	return bind_default_texture(ds,defaultTexName,bindingIndex,optOutTex);
}

bool ShaderPBR::BindDescriptorSetBaseTextures(CMaterial &mat,const prosper::DescriptorSetInfo &descSetInfo,prosper::IDescriptorSet &ds)
{
	auto matData = InitializeMaterialBuffer(ds,mat);
	if(matData.has_value() == false)
		return false;

	Texture *texAlbedo = nullptr;
	if(BindDescriptorSetTexture(mat,ds,mat.GetAlbedoMap(),umath::to_integral(MaterialBinding::AlbedoMap),"white",&texAlbedo) == false)
		return false;

	if(BindDescriptorSetTexture(mat,ds,mat.GetNormalMap(),umath::to_integral(MaterialBinding::NormalMap),"black") == false)
		return false;

	if(BindDescriptorSetTexture(mat,ds,mat.GetRMAMap(),umath::to_integral(MaterialBinding::RMAMap),"pbr/rma_neutral") == false)
		return false;

	BindDescriptorSetTexture(mat,ds,mat.GetGlowMap(),umath::to_integral(MaterialBinding::EmissionMap));

	if(BindDescriptorSetTexture(mat,ds,mat.GetParallaxMap(),umath::to_integral(MaterialBinding::ParallaxMap),"black") == false)
		return false;

	if(BindDescriptorSetTexture(mat,ds,mat.GetTextureInfo("wrinkle_stretch_map"),umath::to_integral(MaterialBinding::WrinkleStretchMap),texAlbedo) == false)
		return false;

	if(BindDescriptorSetTexture(mat,ds,mat.GetTextureInfo("wrinkle_compress_map"),umath::to_integral(MaterialBinding::WrinkleCompressMap),texAlbedo) == false)
		return false;

	if(BindDescriptorSetTexture(mat,ds,mat.GetTextureInfo("exponent_map"),umath::to_integral(MaterialBinding::ExponentMap),"white") == false)
		return false;

	return true;
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
	
	if(BindDescriptorSetBaseTextures(mat,descSetInfo,descSet) == false)
		return nullptr;

	// TODO: FIXME: It would probably be a good idea to update the descriptor set lazily (i.e. not update it here), but
	// that seems to cause crashes in some cases
	if(descSet.Update() == false)
		return nullptr;
	return descSetGroup;
}
void ShaderPBR::OnPipelinesInitialized()
{
	ShaderGameWorldLightingPass::OnPipelinesInitialized();
	auto &context = c_engine->GetRenderContext();
	m_defaultPbrDsg = context.CreateDescriptorSetGroup(pragma::ShaderPBR::DESCRIPTOR_SET_PBR);
	auto &dummyTex = context.GetDummyCubemapTexture();
	auto &ds =* m_defaultPbrDsg->GetDescriptorSet(0);
	ds.SetBindingTexture(*dummyTex,umath::to_integral(PBRBinding::IrradianceMap));
	ds.SetBindingTexture(*dummyTex,umath::to_integral(PBRBinding::PrefilterMap));
	ds.SetBindingTexture(*dummyTex,umath::to_integral(PBRBinding::BRDFMap));
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

//

void ShaderPBR::RecordBindSceneDescriptorSets(
	rendering::ShaderProcessor &shaderProcessor,
	const pragma::CSceneComponent &scene,const pragma::CRasterizationRendererComponent &renderer,
	prosper::IDescriptorSet &dsScene,prosper::IDescriptorSet &dsRenderer,
	prosper::IDescriptorSet &dsRenderSettings,prosper::IDescriptorSet &dsLights,
	prosper::IDescriptorSet &dsShadows,prosper::IDescriptorSet &dsMaterial,
	ShaderGameWorld::SceneFlags &inOutSceneFlags,float &outIblStrength
) const
{
	outIblStrength = 1.f;
	std::array<prosper::IDescriptorSet*,7> descSets {
		&dsMaterial,
		&dsScene,
		&dsRenderer,
		&dsRenderSettings,
		&dsLights,
		&dsShadows,
		GetReflectionProbeDescriptorSet(scene,outIblStrength,inOutSceneFlags)
	};

	static const std::vector<uint32_t> dynamicOffsets {};
	shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics,shaderProcessor.GetCurrentPipelineLayout(),pragma::ShaderGameWorld::MATERIAL_DESCRIPTOR_SET_INDEX,descSets,dynamicOffsets);
}

prosper::IDescriptorSet *ShaderPBR::GetReflectionProbeDescriptorSet(const pragma::CSceneComponent &scene,float &outIblStrength,ShaderGameWorld::SceneFlags &inOutSceneFlags) const
{
	auto &hCam = scene.GetActiveCamera();
	assert(hCam.valid());
	auto *dsPbr = CReflectionProbeComponent::FindDescriptorSetForClosestProbe(scene,hCam->GetEntity().GetPosition(),outIblStrength);
	if(dsPbr == nullptr) // No reflection probe and therefore no IBL available. Fallback to non-IBL rendering.
	{
		dsPbr = &GetDefaultPbrDescriptorSet();
		inOutSceneFlags |= ShaderGameWorld::SceneFlags::NoIBL;
	}
	return dsPbr;
}

void ShaderPBR::RecordBindScene(
	rendering::ShaderProcessor &shaderProcessor,
	const pragma::CSceneComponent &scene,const pragma::CRasterizationRendererComponent &renderer,
	prosper::IDescriptorSet &dsScene,prosper::IDescriptorSet &dsRenderer,
	prosper::IDescriptorSet &dsRenderSettings,prosper::IDescriptorSet &dsLights,
	prosper::IDescriptorSet &dsShadows,prosper::IDescriptorSet &dsMaterial,
	const Vector4 &drawOrigin,ShaderGameWorld::SceneFlags &inOutSceneFlags
) const
{
	auto iblStrength = 1.f;
	RecordBindSceneDescriptorSets(
		shaderProcessor,scene,renderer,
		dsScene,dsRenderer,dsRenderSettings,
		dsLights,dsShadows,dsMaterial,inOutSceneFlags,iblStrength
	);

	ShaderGameWorldLightingPass::PushConstants pushConstants {};
	pushConstants.Initialize();
	pushConstants.debugMode = scene.GetDebugMode();
	pushConstants.reflectionProbeIntensity = iblStrength;
	pushConstants.flags = inOutSceneFlags;
	pushConstants.drawOrigin = drawOrigin;
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(),prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit,0u,sizeof(pushConstants),&pushConstants);
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
	AttachPushConstantRange(pipelineInfo,pipelineIdx,0u,sizeof(ShaderGameWorldLightingPass::PushConstants) +sizeof(PushConstants),prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit);
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
