/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_WATER_SPLASH_HPP__
#define __C_SHADER_WATER_SPLASH_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader.hpp>
#include <pragma/physics/phys_water_surface_simulator.hpp>

namespace pragma {
	class DLLCLIENT ShaderWaterSplash : public prosper::ShaderCompute {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_WATER_EFFECT;
		enum class WaterEffectBinding : uint32_t { WaterParticles = 0u, WaterPositions };

		ShaderWaterSplash(prosper::IPrContext &context, const std::string &identifier);
		bool RecordCompute(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descParticles, const PhysWaterSurfaceSimulator::SplashInfo &info) const;
	  protected:
		virtual void InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

#endif
