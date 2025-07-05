// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_SHADER_COMPUTE_IRRADIANCE_MAP_ROUGHNESS_HPP__
#define __C_SHADER_COMPUTE_IRRADIANCE_MAP_ROUGHNESS_HPP__

#include "pragma/rendering/shaders/c_shader_base_cubemap.hpp"

namespace prosper {
	class Texture;
};
namespace pragma {
	class DLLCLIENT ShaderComputeIrradianceMapRoughness : public ShaderCubemap {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_IRRADIANCE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_ROUGHNESS;

#pragma pack(push, 1)
		struct RoughnessData {
			float roughness;
			float resolution;
		};
#pragma pack(pop)

		ShaderComputeIrradianceMapRoughness(prosper::IPrContext &context, const std::string &identifier);
		std::shared_ptr<prosper::Texture> ComputeRoughness(prosper::Texture &cubemap, uint32_t resolution);
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

#endif
