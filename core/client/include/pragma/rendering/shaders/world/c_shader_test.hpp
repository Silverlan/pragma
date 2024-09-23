/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_TEST_HPP__
#define __C_SHADER_TEST_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

namespace pragma {
	class DLLCLIENT ShaderTest : public ShaderGameWorldLightingPass {
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

			Count
		};

		enum class PBRBinding : uint32_t {
			IrradianceMap = 0u,
			PrefilterMap,
			BRDFMap,

			Count
		};

		ShaderTest(prosper::IPrContext &context, const std::string &identifier);
		ShaderTest(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader = "");

		void DrawTest(prosper::IBuffer &buf, prosper::IBuffer &ibuf, uint32_t count);
		void SetForceNonIBLMode(bool b);
	  protected:
		virtual void InitializeGfxPipelinePushConstantRanges() override;
		virtual void UpdateRenderFlags(CModelSubMesh &mesh, SceneFlags &inOutFlags) override;
		virtual void InitializeGfxPipelineDescriptorSets() override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;

		SceneFlags m_extRenderFlags = SceneFlags::None;
		bool m_bNonIBLMode = false;
		Mat4 m_testMvp;
	};
};

#endif
