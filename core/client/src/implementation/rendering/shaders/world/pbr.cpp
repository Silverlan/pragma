// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :rendering.shaders.pbr;

import :client_state;
import :engine;
import :entities.components;
import :model;

using namespace pragma;

decltype(ShaderPBR::DESCRIPTOR_SET_PBR) ShaderPBR::DESCRIPTOR_SET_PBR = {
  "PBR",
  {prosper::DescriptorSetInfo::Binding {"IRRADIANCE_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit, prosper::PrDescriptorSetBindingFlags::Cubemap},
    prosper::DescriptorSetInfo::Binding {"PREFILTER_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit, prosper::PrDescriptorSetBindingFlags::Cubemap},
    prosper::DescriptorSetInfo::Binding {"BRDF_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
ShaderPBR::ShaderPBR(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader) : ShaderGameWorldLightingPass {context, identifier, vsShader, fsShader, gsShader} {}
ShaderPBR::ShaderPBR(prosper::IPrContext &context, const std::string &identifier) : ShaderPBR {context, identifier, "programs/scene/textured", "programs/scene/pbr/pbr"} {}

void ShaderPBR::UpdateRenderFlags(geometry::CModelSubMesh &mesh, SceneFlags &inOutFlags)
{
	ShaderGameWorldLightingPass::UpdateRenderFlags(mesh, inOutFlags);
	inOutFlags |= m_extRenderFlags;
}
void ShaderPBR::InitializeGfxPipelineDescriptorSets()
{
	ShaderGameWorldLightingPass::InitializeGfxPipelineDescriptorSets();
	AddDescriptorSetGroup(DESCRIPTOR_SET_PBR);
}

void ShaderPBR::InitializeMaterialData(const material::CMaterial &mat, const rendering::shader_material::ShaderMaterial &shaderMat, rendering::ShaderInputData &inOutMatData)
{
	ShaderGameWorldLightingPass::InitializeMaterialData(mat, shaderMat, inOutMatData);
	float specularFactor;
	if(mat.GetProperty("specular_factor", &specularFactor)) {
		auto roughnessFactor = inOutMatData.GetValue<float>("roughness_factor");
		if(!roughnessFactor)
			roughnessFactor = 1.f;
		*roughnessFactor *= (1.f - specularFactor);
		inOutMatData.SetValue<float>("roughness_factor", *roughnessFactor);
	}

#if 0
	auto *glowMap = mat.GetGlowMap();
	auto hasGlowmap = (glowMap != nullptr && glowMap->texture != nullptr);
	if(hasGlowmap || data->HasValue("emission_factor")) {
		auto emissionFactor = get_emission_factor(mat);
		if(emissionFactor) {
			matData.emissionFactor = {emissionFactor->r, emissionFactor->g, emissionFactor->b, 1.f};
			to_srgb_color(matData.emissionFactor);
		}

		if(hasGlowmap || emissionFactor) {
			std::shared_ptr<material::Texture> texture;
			if(hasGlowmap)
				texture = std::static_pointer_cast<material::Texture>(glowMap->texture);
			if(emissionFactor) {
				matFlags |= MaterialFlags::GlowSRGB;
				if(!texture) {
					texture = GetTexture("white");
					hasGlowmap = true;
				}
			}
			auto bUseGlow = true;
			if(hasGlowmap) {
				if(texture->HasFlag(Texture::Flags::SRGB))
					matFlags |= MaterialFlags::GlowSRGB;
				if(data->GetBool("glow_alpha_only") == true) {
					if(prosper::util::has_alpha(texture->GetVkTexture()->GetImage().GetFormat()) == false)
						bUseGlow = false;
				}
			}
			if(bUseGlow == true) {
				int32_t glowMode = 1;
				data->GetInt("glow_blend_diffuse_mode", &glowMode);
				if(glowMode != 0) {
					matFlags |= MaterialFlags::Glow;
					data->GetFloat("glow_blend_diffuse_scale", &matData.glowScale);
				}
				switch(glowMode) {
				case 1:
					matFlags |= MaterialFlags::FMAT_GLOW_MODE_1;
					break;
				case 2:
					matFlags |= MaterialFlags::FMAT_GLOW_MODE_2;
					break;
				case 3:
					matFlags |= MaterialFlags::FMAT_GLOW_MODE_3;
					break;
				case 4:
					matFlags |= MaterialFlags::FMAT_GLOW_MODE_4;
					break;
				}
			}
		}
	}
#endif
}

std::shared_ptr<prosper::IDescriptorSetGroup> ShaderPBR::InitializeMaterialDescriptorSet(material::CMaterial &mat, const prosper::DescriptorSetInfo &descSetInfo) { return ShaderGameWorldLightingPass::InitializeMaterialDescriptorSet(mat, descSetInfo); }
void ShaderPBR::OnPipelinesInitialized()
{
	ShaderGameWorldLightingPass::OnPipelinesInitialized();
	auto &context = get_cengine()->GetRenderContext();
	m_defaultPbrDsg = context.CreateDescriptorSetGroup(DESCRIPTOR_SET_PBR);
	auto &dummyTex = context.GetDummyTexture();
	auto &dummyCubemapTex = context.GetDummyCubemapTexture();
	auto &ds = *m_defaultPbrDsg->GetDescriptorSet(0);
	ds.SetBindingTexture(*dummyCubemapTex, math::to_integral(PBRBinding::IrradianceMap));
	ds.SetBindingTexture(*dummyCubemapTex, math::to_integral(PBRBinding::PrefilterMap));
	ds.SetBindingTexture(*dummyTex, math::to_integral(PBRBinding::BRDFMap));
}
prosper::IDescriptorSet &ShaderPBR::GetDefaultPbrDescriptorSet() const { return *m_defaultPbrDsg->GetDescriptorSet(); }
void ShaderPBR::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderGameWorldLightingPass::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }

//

void ShaderPBR::RecordBindSceneDescriptorSets(rendering::ShaderProcessor &shaderProcessor, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer,
  prosper::IDescriptorSet &dsRenderSettings, prosper::IDescriptorSet &dsShadows, SceneFlags &inOutSceneFlags, float &outIblStrength) const
{
	outIblStrength = 1.f;
	std::array<prosper::IDescriptorSet *, 5> descSets {&dsScene, &dsRenderer, &dsRenderSettings, &dsShadows, GetReflectionProbeDescriptorSet(scene, outIblStrength, inOutSceneFlags)};

	static const std::vector<uint32_t> dynamicOffsets {};
	shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, shaderProcessor.GetCurrentPipelineLayout(), GetSceneDescriptorSetIndex(), descSets, dynamicOffsets);
}

prosper::IDescriptorSet *ShaderPBR::GetReflectionProbeDescriptorSet(const CSceneComponent &scene, float &outIblStrength, SceneFlags &inOutSceneFlags) const
{
	auto &hCam = scene.GetActiveCamera();
	assert(hCam.valid());
	auto *dsPbr = CReflectionProbeComponent::FindDescriptorSetForClosestProbe(scene, hCam->GetEntity().GetPosition(), outIblStrength);
	if(dsPbr == nullptr) // No reflection probe and therefore no IBL available. Fallback to non-IBL rendering.
	{
		dsPbr = &GetDefaultPbrDescriptorSet();
		inOutSceneFlags |= SceneFlags::NoIBL;
	}
	return dsPbr;
}

void ShaderPBR::RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
  prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, SceneFlags &inOutSceneFlags) const
{
	auto iblStrength = 1.f;
	RecordBindSceneDescriptorSets(shaderProcessor, scene, renderer, dsScene, dsRenderer, dsRenderSettings, dsShadows, inOutSceneFlags, iblStrength);

	PushConstants pushConstants {};
	pushConstants.Initialize();
	pushConstants.debugMode = scene.GetDebugMode();
	pushConstants.reflectionProbeIntensity = iblStrength;
	pushConstants.flags = inOutSceneFlags;
	pushConstants.drawOrigin = drawOrigin;
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, 0u, sizeof(pushConstants), &pushConstants);
}
