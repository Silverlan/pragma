/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_WATER_HPP__
#define __C_SHADER_WATER_HPP__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include <pragma/types.hpp>

namespace pragma {
	class DLLCLIENT ShaderWater : public ShaderGameWorldLightingPass {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_WATER;
		enum class MaterialBinding : uint32_t {
			DuDvMap = 0u,
			NormalMap,

			Count
		};

		enum class WaterBinding : uint32_t {
			ReflectionMap = 0u,
			RefractionMap,
			RefractionDepth,
			WaterSettings,
			WaterFog,

			Count
		};

#pragma pack(push, 1)
		struct PushConstants {
			float waterFogIntensity;
			uint32_t enableReflection;
		};
#pragma pack(pop)

		ShaderWater(prosper::IPrContext &context, const std::string &identifier);
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;

		virtual bool RecordBindEntity(rendering::ShaderProcessor &shaderProcessor, CRenderComponent &renderC, prosper::IShaderPipelineLayout &layout, uint32_t entityInstanceDescriptorSetIndex) const override;
		virtual GameShaderSpecializationConstantFlag GetBaseSpecializationFlags() const override;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipelineDescriptorSets() override;
		virtual void InitializeGfxPipelinePushConstantRanges() override;
		virtual prosper::DescriptorSetInfo &GetMaterialDescriptorSetInfo() const;
	  private:
		bool UpdateBindFogDensity();
	};
};

#endif
