// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.specular_glossiness_to_metalness_roughness;

export import pragma.prosper;

export namespace pragma {
	class DLLCLIENT ShaderSpecularGlossinessToMetalnessRoughness : public prosper::ShaderBaseImageProcessing {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;

		enum class TextureBinding : uint32_t {
			DiffuseMap = 0,
			SpecularGlossinessMap,
			AmbientOcclusionMap,

			Count
		};

		struct DLLCLIENT MetalnessRoughnessImageSet {
			std::shared_ptr<prosper::IImage> albedoMap = nullptr;
			std::shared_ptr<prosper::IImage> rmaMap = nullptr;
		};

		enum class Pass : uint32_t { Albedo = 0, RMA };

#pragma pack(push, 1)
		struct PushConstants {

			PushConstants() {};
			Vector4 diffuseFactor = {1.f, 1.f, 1.f, 1.f};
			Vector4 specularFactor = {1.f, 1.f, 1.f, 1.f}; // Alpha is glossiness factor
			Pass pass = Pass::Albedo;
		};
#pragma pack(pop)

		ShaderSpecularGlossinessToMetalnessRoughness(prosper::IPrContext &context, const std::string &identifier);
		std::optional<MetalnessRoughnessImageSet> ConvertToMetalnessRoughness(prosper::IPrContext &context, prosper::Texture *optDiffuseMap, prosper::Texture *optSpecularGlossinessMap, const PushConstants &pushConstants = {}, prosper::Texture *optAoMap = nullptr);
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};
