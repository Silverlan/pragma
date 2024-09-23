/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2023 Silverlan
 */

#ifndef __C_SHADER_GLOW_HPP__
#define __C_SHADER_GLOW_HPP__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

class Texture;
namespace pragma {
	class DLLCLIENT ShaderGlow : public ShaderGameWorldLightingPass {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_PBR;

		enum class MaterialBinding : uint32_t {
			MaterialSettings = umath::to_integral(ShaderGameWorldLightingPass::MaterialBinding::MaterialSettings),
			AlbedoMap,
			NormalMap,
			RMAMap,
			EmissionMap,
			ParallaxMap,
			WrinkleStretchMap,
			WrinkleCompressMap,
			ExponentMap,
			GlowMap,

			Count
		};

		enum class PBRBinding : uint32_t {
			IrradianceMap = 0u,
			PrefilterMap,
			BRDFMap,

			Count
		};

		ShaderGlow(prosper::IPrContext &context, const std::string &identifier);
		ShaderGlow(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader = "");

		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
		prosper::IDescriptorSet &GetDefaultPbrDescriptorSet() const;

		//
		virtual void RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
		  prosper::IDescriptorSet &dsLights, prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, ShaderGameWorld::SceneFlags &inOutSceneFlags) const override;

		static bool BindDescriptorSetTexture(Material &mat, prosper::IDescriptorSet &ds, TextureInfo *texInfo, uint32_t bindingIndex, const std::string &defaultTexName, Texture **optOutTex = nullptr);
		static bool BindDescriptorSetTexture(Material &mat, prosper::IDescriptorSet &ds, TextureInfo *texInfo, uint32_t bindingIndex, Texture *optDefaultTex = nullptr);
		prosper::IDescriptorSet *GetReflectionProbeDescriptorSet(const pragma::CSceneComponent &scene, float &outIblStrength, ShaderGameWorld::SceneFlags &inOutSceneFlags) const;
	  protected:
		using ShaderGameWorldLightingPass::RecordDraw;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		void RecordBindSceneDescriptorSets(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer,
		  prosper::IDescriptorSet &dsRenderSettings, prosper::IDescriptorSet &dsLights, prosper::IDescriptorSet &dsShadows, ShaderGameWorld::SceneFlags &inOutSceneFlags, float &outIblStrength) const;
		virtual void OnPipelinesInitialized() override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void UpdateRenderFlags(CModelSubMesh &mesh, SceneFlags &inOutFlags) override;
		virtual void InitializeGfxPipelineDescriptorSets() override;
		std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat, const prosper::DescriptorSetInfo &descSetInfo);
		bool BindDescriptorSetBaseTextures(CMaterial &mat, const prosper::DescriptorSetInfo &descSetInfo, prosper::IDescriptorSet &ds);

		SceneFlags m_extRenderFlags = SceneFlags::None;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_defaultPbrDsg = nullptr;
	};
};

#endif
