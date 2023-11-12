/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_EYE_HPP__
#define __C_SHADER_EYE_HPP__

#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"

namespace pragma {
	namespace rendering {
		class ShaderProcessor;
	};
	class DLLCLIENT ShaderEye : public ShaderPBR {
	  public:
#pragma pack(push, 1)
		struct PushConstants {
			Vector4 irisProjectionU = {};
			Vector4 irisProjectionV = {};
			Vector3 eyeOrigin = {};
			float irisUvClampThreshold = 0.5f; // Only used in legacy shader

			float maxDilationFactor = 1.f;
			float dilationFactor = 0.5f;
			float irisUvRadius = 0.2f;
		};
#pragma pack(pop)

		ShaderEye(prosper::IPrContext &context, const std::string &identifier);

		//
		virtual void RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
		  prosper::IDescriptorSet &dsLights, prosper::IDescriptorSet &dsShadows, prosper::IDescriptorSet &dsMaterial, const Vector4 &drawOrigin, ShaderGameWorld::SceneFlags &inOutSceneFlags) const override;
		virtual bool OnRecordDrawMesh(rendering::ShaderProcessor &shaderProcessor, CModelSubMesh &mesh) const override;
	  protected:
		ShaderEye(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader = "");
		bool BindEyeball(rendering::ShaderProcessor &shaderProcessor, uint32_t skinMatIdx) const;
		virtual void InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
	};

	class DLLCLIENT ShaderEyeLegacy : public ShaderEye {
	  public:
		ShaderEyeLegacy(prosper::IPrContext &context, const std::string &identifier);
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
		virtual bool RecordBindMaterial(rendering::ShaderProcessor &shaderProcessor, CMaterial &mat) const override;
	  private:
	};
};

#endif
