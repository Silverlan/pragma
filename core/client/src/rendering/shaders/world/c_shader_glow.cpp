/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_glow.hpp"
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

decltype(ShaderGlow::DESCRIPTOR_SET_PBR) ShaderGlow::DESCRIPTOR_SET_PBR = {
  "PBR",
  {
    prosper::DescriptorSetInfo::Binding {"IRRADIANCE_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit, prosper::PrDescriptorSetBindingFlags::Cubemap},
    prosper::DescriptorSetInfo::Binding {"PREFILTER_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit, prosper::PrDescriptorSetBindingFlags::Cubemap},
    prosper::DescriptorSetInfo::Binding {"BRDF_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
  },
};
ShaderGlow::ShaderGlow(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader) : ShaderGameWorldLightingPass {context, identifier, vsShader, fsShader, gsShader} { m_shaderMaterialName = "pbr"; }
ShaderGlow::ShaderGlow(prosper::IPrContext &context, const std::string &identifier) : ShaderGlow {context, identifier, "programs/scene/textured", "programs/scene/glow"} {}

void ShaderGlow::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	auto sampleCount = GetSampleCount(pipelineIdx);
	prosper::util::RenderPassCreateInfo rpCreateInfo {
	  {{RENDER_PASS_FORMAT, prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::DontCare, prosper::AttachmentStoreOp::Store, sampleCount, prosper::ImageLayout::ColorAttachmentOptimal},
	    {RENDER_PASS_FORMAT, prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::Clear, prosper::AttachmentStoreOp::Store, sampleCount, prosper::ImageLayout::ColorAttachmentOptimal}, // Bloom Attachment
	    {RENDER_PASS_DEPTH_FORMAT, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::AttachmentLoadOp::Load, prosper::AttachmentStoreOp::Store /* depth values have already been written by prepass */, sampleCount, prosper::ImageLayout::DepthStencilAttachmentOptimal}},
	};
	rpCreateInfo.subPasses.push_back(prosper::util::RenderPassCreateInfo::SubPass {std::vector<std::size_t> {0ull, 1ull}, true});

	CreateCachedRenderPass<ShaderScene>(rpCreateInfo, outRenderPass, pipelineIdx);
}

void ShaderGlow::UpdateRenderFlags(CModelSubMesh &mesh, SceneFlags &inOutFlags)
{
	ShaderGameWorldLightingPass::UpdateRenderFlags(mesh, inOutFlags);
	inOutFlags |= m_extRenderFlags;
}
void ShaderGlow::InitializeGfxPipelineDescriptorSets()
{
	ShaderGameWorldLightingPass::InitializeGfxPipelineDescriptorSets();
	AddDescriptorSetGroup(DESCRIPTOR_SET_PBR);
}

bool ShaderGlow::BindDescriptorSetTexture(Material &mat, prosper::IDescriptorSet &ds, TextureInfo *texInfo, uint32_t bindingIndex, Texture *optDefaultTex)
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

static bool bind_default_texture(prosper::IDescriptorSet &ds, const std::string &defaultTexName, uint32_t bindingIndex, Texture **optOutTex)
{
	auto tex = pragma::ShaderGameWorldLightingPass::GetTexture(defaultTexName);
	if(!tex) {
		Con::cwar << "Attempted to bind texture '" << defaultTexName << "' to material descriptor set, but texture has not been loaded!" << Con::endl;
		return false;
	}

	if(tex && tex->HasValidVkTexture())
		ds.SetBindingTexture(*tex->GetVkTexture(), bindingIndex);
	if(optOutTex)
		*optOutTex = tex.get();
	return true;
}

bool ShaderGlow::BindDescriptorSetTexture(Material &mat, prosper::IDescriptorSet &ds, TextureInfo *texInfo, uint32_t bindingIndex, const std::string &defaultTexName, Texture **optOutTex)
{
	auto &matManager = static_cast<msys::CMaterialManager &>(client->GetMaterialManager());
	auto &texManager = matManager.GetTextureManager();

	std::shared_ptr<Texture> tex = nullptr;
	if(texInfo && texInfo->texture) {
		tex = std::static_pointer_cast<Texture>(texInfo->texture);
		if(tex->HasValidVkTexture()) {
			ds.SetBindingTexture(*tex->GetVkTexture(), bindingIndex);
			if(optOutTex)
				*optOutTex = tex.get();
			return true;
		}
	}
	else if(defaultTexName.empty())
		return false;
	return bind_default_texture(ds, defaultTexName, bindingIndex, optOutTex);
}

bool ShaderGlow::BindDescriptorSetBaseTextures(CMaterial &mat, const prosper::DescriptorSetInfo &descSetInfo, prosper::IDescriptorSet &ds)
{
#if 0
	auto matData = InitializeMaterialBuffer(ds, mat);
	if(matData.has_value() == false)
		return false;

	Texture *texAlbedo = nullptr;
	if(BindDescriptorSetTexture(mat, ds, mat.GetAlbedoMap(), umath::to_integral(MaterialBinding::AlbedoMap), "white", &texAlbedo) == false)
		return false;

	if(BindDescriptorSetTexture(mat, ds, mat.GetNormalMap(), umath::to_integral(MaterialBinding::NormalMap), "black") == false)
		return false;

	if(BindDescriptorSetTexture(mat, ds, mat.GetRMAMap(), umath::to_integral(MaterialBinding::RMAMap), "pbr/rma_neutral") == false)
		return false;

	if(umath::is_flag_set(matData->flags, ShaderGameWorldLightingPass::MaterialFlags::Glow))
		BindDescriptorSetTexture(mat, ds, mat.GetGlowMap(), umath::to_integral(MaterialBinding::EmissionMap), "white");
	else
		BindDescriptorSetTexture(mat, ds, mat.GetGlowMap(), umath::to_integral(MaterialBinding::EmissionMap));

	if(BindDescriptorSetTexture(mat, ds, mat.GetParallaxMap(), umath::to_integral(MaterialBinding::ParallaxMap), "black") == false)
		return false;

	if(BindDescriptorSetTexture(mat, ds, mat.GetTextureInfo("wrinkle_stretch_map"), umath::to_integral(MaterialBinding::WrinkleStretchMap), texAlbedo) == false)
		return false;

	if(BindDescriptorSetTexture(mat, ds, mat.GetTextureInfo("wrinkle_compress_map"), umath::to_integral(MaterialBinding::WrinkleCompressMap), texAlbedo) == false)
		return false;

	if(BindDescriptorSetTexture(mat, ds, mat.GetTextureInfo("exponent_map"), umath::to_integral(MaterialBinding::ExponentMap), "white") == false)
		return false;

	if(BindDescriptorSetTexture(mat, ds, mat.GetTextureInfo("glow_map"), umath::to_integral(MaterialBinding::GlowMap), "white") == false)
		return false;

	return true;
#endif
	return false;
}

std::shared_ptr<prosper::IDescriptorSetGroup> ShaderGlow::InitializeMaterialDescriptorSet(CMaterial &mat, const prosper::DescriptorSetInfo &descSetInfo)
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

	if(BindDescriptorSetBaseTextures(mat, descSetInfo, descSet) == false)
		return nullptr;

	// TODO: FIXME: It would probably be a good idea to update the descriptor set lazily (i.e. not update it here), but
	// that seems to cause crashes in some cases
	if(descSet.Update() == false)
		return nullptr;
	return descSetGroup;
}
void ShaderGlow::OnPipelinesInitialized()
{
	ShaderGameWorldLightingPass::OnPipelinesInitialized();
	auto &context = c_engine->GetRenderContext();
	m_defaultPbrDsg = context.CreateDescriptorSetGroup(pragma::ShaderGlow::DESCRIPTOR_SET_PBR);
	auto &dummyTex = context.GetDummyTexture();
	auto &dummyCubemapTex = context.GetDummyCubemapTexture();
	auto &ds = *m_defaultPbrDsg->GetDescriptorSet(0);
	ds.SetBindingTexture(*dummyCubemapTex, umath::to_integral(PBRBinding::IrradianceMap));
	ds.SetBindingTexture(*dummyCubemapTex, umath::to_integral(PBRBinding::PrefilterMap));
	ds.SetBindingTexture(*dummyTex, umath::to_integral(PBRBinding::BRDFMap));
}
prosper::IDescriptorSet &ShaderGlow::GetDefaultPbrDescriptorSet() const { return *m_defaultPbrDsg->GetDescriptorSet(); }
std::shared_ptr<prosper::IDescriptorSetGroup> ShaderGlow::InitializeMaterialDescriptorSet(CMaterial &mat) { return InitializeMaterialDescriptorSet(mat, GetMaterialDescriptorSetInfo()); }
void ShaderGlow::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderGameWorldLightingPass::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }

//

void ShaderGlow::RecordBindSceneDescriptorSets(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer,
  prosper::IDescriptorSet &dsRenderSettings, prosper::IDescriptorSet &dsLights, prosper::IDescriptorSet &dsShadows, ShaderGameWorld::SceneFlags &inOutSceneFlags, float &outIblStrength) const
{
	outIblStrength = 1.f;
	std::array<prosper::IDescriptorSet *, 6> descSets {&dsScene, &dsRenderer, &dsRenderSettings, &dsLights, &dsShadows, GetReflectionProbeDescriptorSet(scene, outIblStrength, inOutSceneFlags)};

	static const std::vector<uint32_t> dynamicOffsets {};
	shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, shaderProcessor.GetCurrentPipelineLayout(), GetSceneDescriptorSetIndex(), descSets, dynamicOffsets);
}

prosper::IDescriptorSet *ShaderGlow::GetReflectionProbeDescriptorSet(const pragma::CSceneComponent &scene, float &outIblStrength, ShaderGameWorld::SceneFlags &inOutSceneFlags) const
{
	auto &hCam = scene.GetActiveCamera();
	assert(hCam.valid());
	auto *dsPbr = CReflectionProbeComponent::FindDescriptorSetForClosestProbe(scene, hCam->GetEntity().GetPosition(), outIblStrength);
	if(dsPbr == nullptr) // No reflection probe and therefore no IBL available. Fallback to non-IBL rendering.
	{
		dsPbr = &GetDefaultPbrDescriptorSet();
		inOutSceneFlags |= ShaderGameWorld::SceneFlags::NoIBL;
	}
	return dsPbr;
}

void ShaderGlow::RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
  prosper::IDescriptorSet &dsLights, prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, ShaderGameWorld::SceneFlags &inOutSceneFlags) const
{
	auto iblStrength = 1.f;
	RecordBindSceneDescriptorSets(shaderProcessor, scene, renderer, dsScene, dsRenderer, dsRenderSettings, dsLights, dsShadows, inOutSceneFlags, iblStrength);

	ShaderGameWorldLightingPass::PushConstants pushConstants {};
	pushConstants.Initialize();
	pushConstants.debugMode = scene.GetDebugMode();
	pushConstants.reflectionProbeIntensity = iblStrength;
	pushConstants.flags = inOutSceneFlags;
	pushConstants.drawOrigin = drawOrigin;
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, 0u, sizeof(pushConstants), &pushConstants);
}
