// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_SHADER_WATER_SURFACE_SUM_EDGES_HPP__
#define __C_SHADER_WATER_SURFACE_SUM_EDGES_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader.hpp>

namespace pragma {
	class DLLCLIENT ShaderWaterSurfaceSumEdges : public prosper::ShaderCompute {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_WATER;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SURFACE_INFO;

		enum class WaterBinding : uint32_t { WaterParticles = 0u, WaterEdgeData };

		ShaderWaterSurfaceSumEdges(prosper::IPrContext &context, const std::string &identifier);
		bool RecordCompute(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetSurfaceInfo, prosper::IDescriptorSet &descSetEdges, uint32_t width, uint32_t length) const;
	  protected:
		ShaderWaterSurfaceSumEdges(prosper::IPrContext &context, const std::string &identifier, const std::string &csShader);
		virtual void InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

#endif
