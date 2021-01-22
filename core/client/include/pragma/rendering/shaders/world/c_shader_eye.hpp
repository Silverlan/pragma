/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_EYE_HPP__
#define __C_SHADER_EYE_HPP__

#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"

namespace pragma
{
	namespace rendering {class ShaderProcessor;};
	class DLLCLIENT ShaderEye
		: public ShaderPBR
	{
	public:
#pragma pack(push,1)
		struct PushConstants
		{
			Vector4 irisProjectionU = {};
			Vector4 irisProjectionV = {};
			Vector4 eyeOrigin = {};

			float maxDilationFactor = 1.f;
			float dilationFactor = 0.5f;
			float irisUvRadius = 0.2f;
		};
#pragma pack(pop)

		ShaderEye(prosper::IPrContext &context,const std::string &identifier);
		virtual bool Draw(CModelSubMesh &mesh,const std::optional<pragma::RenderMeshIndex> &meshIdx,prosper::IBuffer &renderBufferIndexBuffer,uint32_t instanceCount=1) override;

		bool Draw(pragma::rendering::ShaderProcessor &shaderProcessor);

		//
		virtual void RecordBindScene(
			rendering::ShaderProcessor &shaderProcessor,
			const pragma::CSceneComponent &scene,const pragma::CRasterizationRendererComponent &renderer,
			prosper::IDescriptorSet &dsScene,prosper::IDescriptorSet &dsRenderer,
			prosper::IDescriptorSet &dsRenderSettings,prosper::IDescriptorSet &dsLights,
			prosper::IDescriptorSet &dsShadows,prosper::IDescriptorSet &dsMaterial,
			ShaderGameWorld::SceneFlags &inOutSceneFlags
		) const override;
		virtual void RecordSceneFlags(rendering::ShaderProcessor &shaderProcessor,SceneFlags sceneFlags) const override {}
		virtual void RecordClipPlane(rendering::ShaderProcessor &shaderProcessor,const Vector4 &clipPlane) const override {}
		virtual void OnRecordDrawMesh(rendering::ShaderProcessor &shaderProcessor,CModelSubMesh &mesh) const override;
	protected:
		bool BindEyeball(rendering::ShaderProcessor &shaderProcessor,uint32_t skinMatIdx) const;
		virtual void InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

#endif
