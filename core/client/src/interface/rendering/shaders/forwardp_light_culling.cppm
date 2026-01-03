// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.forwardp_light_culling;

export import pragma.prosper;

export namespace pragma {
	class DLLCLIENT ShaderForwardPLightCulling : public prosper::ShaderCompute {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_LIGHTS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;

		static uint32_t TILE_SIZE;

		enum class CameraBinding : uint32_t { Camera = 0u, RenderSettings };

		enum class LightBinding : uint32_t { LightBuffers = 0u, TileVisLightIndexBuffer, ShadowData, VisLightIndexBuffer, DepthMap };

#pragma pack(push, 1)
		struct PushConstants {
			uint32_t numLights;
			uint32_t sceneFlag;
			uint32_t viewportResolution;
		};
#pragma pack(pop)

		ShaderForwardPLightCulling(prosper::IPrContext &context, const std::string &identifier);
		bool RecordCompute(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetLights, prosper::IDescriptorSet &descSetCamera, uint32_t vpWidth, uint32_t vpHeight, uint32_t workGroupsX, uint32_t workGroupsY, uint32_t lightCount, uint32_t sceneIndex) const;
	  protected:
		virtual void InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};
