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
#include "pragma/rendering/shader_material/shader_material.hpp"
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

decltype(ShaderPBR::DESCRIPTOR_SET_PBR) ShaderPBR::DESCRIPTOR_SET_PBR = {
  "PBR",
  {prosper::DescriptorSetInfo::Binding {"IRRADIANCE_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit, prosper::PrDescriptorSetBindingFlags::Cubemap},
    prosper::DescriptorSetInfo::Binding {"PREFILTER_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit, prosper::PrDescriptorSetBindingFlags::Cubemap},
    prosper::DescriptorSetInfo::Binding {"BRDF_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
ShaderPBR::ShaderPBR(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader) : ShaderGameWorldLightingPass {context, identifier, vsShader, fsShader, gsShader} {}
ShaderPBR::ShaderPBR(prosper::IPrContext &context, const std::string &identifier) : ShaderPBR {context, identifier, "programs/scene/textured", "programs/scene/pbr/pbr"} {}

void ShaderPBR::UpdateRenderFlags(CModelSubMesh &mesh, SceneFlags &inOutFlags)
{
	ShaderGameWorldLightingPass::UpdateRenderFlags(mesh, inOutFlags);
	inOutFlags |= m_extRenderFlags;
}
void ShaderPBR::InitializeGfxPipelineDescriptorSets()
{
	ShaderGameWorldLightingPass::InitializeGfxPipelineDescriptorSets();
	AddDescriptorSetGroup(DESCRIPTOR_SET_PBR);
}

void ShaderPBR::InitializeMaterialData(const CMaterial &mat, const rendering::shader_material::ShaderMaterial &shaderMat, pragma::rendering::ShaderInputData &inOutMatData)
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
			std::shared_ptr<Texture> texture;
			if(hasGlowmap)
				texture = std::static_pointer_cast<Texture>(glowMap->texture);
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

std::shared_ptr<prosper::IDescriptorSetGroup> ShaderPBR::InitializeMaterialDescriptorSet(CMaterial &mat, const prosper::DescriptorSetInfo &descSetInfo) { return ShaderGameWorldLightingPass::InitializeMaterialDescriptorSet(mat, descSetInfo); }
void ShaderPBR::OnPipelinesInitialized()
{
	ShaderGameWorldLightingPass::OnPipelinesInitialized();
	auto &context = c_engine->GetRenderContext();
	m_defaultPbrDsg = context.CreateDescriptorSetGroup(pragma::ShaderPBR::DESCRIPTOR_SET_PBR);
	auto &dummyTex = context.GetDummyTexture();
	auto &dummyCubemapTex = context.GetDummyCubemapTexture();
	auto &ds = *m_defaultPbrDsg->GetDescriptorSet(0);
	ds.SetBindingTexture(*dummyCubemapTex, umath::to_integral(PBRBinding::IrradianceMap));
	ds.SetBindingTexture(*dummyCubemapTex, umath::to_integral(PBRBinding::PrefilterMap));
	ds.SetBindingTexture(*dummyTex, umath::to_integral(PBRBinding::BRDFMap));
}
prosper::IDescriptorSet &ShaderPBR::GetDefaultPbrDescriptorSet() const { return *m_defaultPbrDsg->GetDescriptorSet(); }
void ShaderPBR::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderGameWorldLightingPass::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }

//

void ShaderPBR::RecordBindSceneDescriptorSets(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer,
  prosper::IDescriptorSet &dsRenderSettings, prosper::IDescriptorSet &dsShadows, ShaderGameWorld::SceneFlags &inOutSceneFlags, float &outIblStrength) const
{
	outIblStrength = 1.f;
	std::array<prosper::IDescriptorSet *, 5> descSets {&dsScene, &dsRenderer, &dsRenderSettings, &dsShadows, GetReflectionProbeDescriptorSet(scene, outIblStrength, inOutSceneFlags)};

	static const std::vector<uint32_t> dynamicOffsets {};
	shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, shaderProcessor.GetCurrentPipelineLayout(), GetSceneDescriptorSetIndex(), descSets, dynamicOffsets);
}

prosper::IDescriptorSet *ShaderPBR::GetReflectionProbeDescriptorSet(const pragma::CSceneComponent &scene, float &outIblStrength, ShaderGameWorld::SceneFlags &inOutSceneFlags) const
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

void ShaderPBR::RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
  prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, ShaderGameWorld::SceneFlags &inOutSceneFlags) const
{
	auto iblStrength = 1.f;
	RecordBindSceneDescriptorSets(shaderProcessor, scene, renderer, dsScene, dsRenderer, dsRenderSettings, dsShadows, inOutSceneFlags, iblStrength);

	ShaderGameWorldLightingPass::PushConstants pushConstants {};
	pushConstants.Initialize();
	pushConstants.debugMode = scene.GetDebugMode();
	pushConstants.reflectionProbeIntensity = iblStrength;
	pushConstants.flags = inOutSceneFlags;
	pushConstants.drawOrigin = drawOrigin;
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, 0u, sizeof(pushConstants), &pushConstants);
}

/////////////////

decltype(ShaderPBRBlend::VERTEX_BINDING_ALPHA) ShaderPBRBlend::VERTEX_BINDING_ALPHA = {prosper::VertexInputRate::Vertex};
decltype(ShaderPBRBlend::VERTEX_ATTRIBUTE_ALPHA) ShaderPBRBlend::VERTEX_ATTRIBUTE_ALPHA = {VERTEX_BINDING_ALPHA, prosper::Format::R32G32_SFloat};
ShaderPBRBlend::ShaderPBRBlend(prosper::IPrContext &context, const std::string &identifier) : ShaderPBR {context, identifier, "programs/scene/textured_blend", "programs/scene/pbr/pbr_blend"} { m_shaderMaterialName = "pbr_blend"; }
void ShaderPBRBlend::InitializeGfxPipelineVertexAttributes()
{
	ShaderPBR::InitializeGfxPipelineVertexAttributes();
	AddVertexAttribute(VERTEX_ATTRIBUTE_ALPHA);
}
void ShaderPBRBlend::InitializeGfxPipelinePushConstantRanges() { AttachPushConstantRange(0u, sizeof(ShaderGameWorldLightingPass::PushConstants) + sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit); }
bool ShaderPBRBlend::GetRenderBufferTargets(CModelSubMesh &mesh, uint32_t pipelineIdx, std::vector<prosper::IBuffer *> &outBuffers, std::vector<prosper::DeviceSize> &outOffsets, std::optional<prosper::IndexBufferInfo> &outIndexBufferInfo) const
{
	if(ShaderPBR::GetRenderBufferTargets(mesh, pipelineIdx, outBuffers, outOffsets, outIndexBufferInfo) == false)
		return false;
	auto &sceneMesh = mesh.GetSceneMesh();
	auto *alphaBuf = sceneMesh->GetAlphaBuffer().get();
	outBuffers.push_back(alphaBuf);
	outOffsets.push_back(0ull);
	return true;
}
bool ShaderPBRBlend::OnRecordDrawMesh(rendering::ShaderProcessor &shaderProcessor, CModelSubMesh &mesh) const
{
	PushConstants pushConstants {};
	pushConstants.alphaCount = mesh.GetAlphaCount();
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, sizeof(ShaderGameWorldLightingPass::PushConstants), sizeof(pushConstants), &pushConstants);
	return ShaderPBR::OnRecordDrawMesh(shaderProcessor, mesh);
}
