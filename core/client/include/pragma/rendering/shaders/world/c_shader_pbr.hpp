/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_PBR_HPP__
#define __C_SHADER_PBR_HPP__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

class Texture;
namespace pragma {
	class DLLCLIENT ShaderPBR : public ShaderGameWorldLightingPass {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;
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

			Count
		};

		enum class PBRBinding : uint32_t {
			IrradianceMap = 0u,
			PrefilterMap,
			BRDFMap,

			Count
		};

		ShaderPBR(prosper::IPrContext &context, const std::string &identifier);
		ShaderPBR(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader = "");

		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
		prosper::IDescriptorSet &GetDefaultPbrDescriptorSet() const;

		//
		virtual void RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
		  prosper::IDescriptorSet &dsLights, prosper::IDescriptorSet &dsShadows, prosper::IDescriptorSet &dsMaterial, const Vector4 &drawOrigin, ShaderGameWorld::SceneFlags &inOutSceneFlags) const override;

		static bool BindDescriptorSetTexture(Material &mat, prosper::IDescriptorSet &ds, TextureInfo *texInfo, uint32_t bindingIndex, const std::string &defaultTexName, Texture **optOutTex = nullptr);
		static bool BindDescriptorSetTexture(Material &mat, prosper::IDescriptorSet &ds, TextureInfo *texInfo, uint32_t bindingIndex, Texture *optDefaultTex = nullptr);
		prosper::IDescriptorSet *GetReflectionProbeDescriptorSet(const pragma::CSceneComponent &scene, float &outIblStrength, ShaderGameWorld::SceneFlags &inOutSceneFlags) const;
	  protected:
		using ShaderGameWorldLightingPass::RecordDraw;
		void RecordBindSceneDescriptorSets(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer,
		  prosper::IDescriptorSet &dsRenderSettings, prosper::IDescriptorSet &dsLights, prosper::IDescriptorSet &dsShadows, prosper::IDescriptorSet &dsMaterial, ShaderGameWorld::SceneFlags &inOutSceneFlags, float &outIblStrength) const;
		virtual void OnPipelinesInitialized() override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void UpdateRenderFlags(CModelSubMesh &mesh, SceneFlags &inOutFlags) override;
		virtual prosper::DescriptorSetInfo &GetMaterialDescriptorSetInfo() const override;
		virtual void InitializeGfxPipelineDescriptorSets(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat, const prosper::DescriptorSetInfo &descSetInfo);
		bool BindDescriptorSetBaseTextures(CMaterial &mat, const prosper::DescriptorSetInfo &descSetInfo, prosper::IDescriptorSet &ds);

		SceneFlags m_extRenderFlags = SceneFlags::None;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_defaultPbrDsg = nullptr;
	};

	class DLLCLIENT ShaderPBRBlend : public ShaderPBR {
	  public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_ALPHA;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_ALPHA;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;
		enum class MaterialBinding : uint32_t { AlbedoMap2 = umath::to_integral(ShaderPBR::MaterialBinding::Count), AlbedoMap3 };

#pragma pack(push, 1)
		struct PushConstants {
			int32_t alphaCount;
		};
#pragma pack(pop)

		ShaderPBRBlend(prosper::IPrContext &context, const std::string &identifier);
		virtual bool GetRenderBufferTargets(CModelSubMesh &mesh, uint32_t pipelineIdx, std::vector<prosper::IBuffer *> &outBuffers, std::vector<prosper::DeviceSize> &outOffsets, std::optional<prosper::IndexBufferInfo> &outIndexBufferInfo) const override;
	  protected:
		virtual void InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipelineVertexAttributes(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx);
		virtual prosper::DescriptorSetInfo &GetMaterialDescriptorSetInfo() const override;
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
	};
};

#endif
