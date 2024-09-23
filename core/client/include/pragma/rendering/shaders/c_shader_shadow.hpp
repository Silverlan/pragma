/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_SHADOW_HPP__
#define __C_SHADER_SHADOW_HPP__

#include "pragma/rendering/shaders/world/c_shader_scene.hpp"

namespace pragma {
	class CLightPointComponent;
	class CLightSpotComponent;
	class CLightComponent;
	class DLLCLIENT ShaderShadow : public ShaderGameWorld {
	  public:
		static prosper::Format RENDER_PASS_DEPTH_FORMAT;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_RENDER_BUFFER_INDEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_BONE_WEIGHT;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT_ID;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_BONE_WEIGHT_EXT;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_POSITION;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_INSTANCE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDER_SETTINGS;

		enum class Pipeline : uint32_t {
			WithMorphTargetAnimations,
			Default = WithMorphTargetAnimations,

			Count
		};

#pragma pack(push, 1)
		struct PushConstants {
			void Initialize()
			{
				depthMVP = umat::identity();
				lightPos = {};
				flags = SceneFlags::None;
			}
			Mat4 depthMVP;
			Vector4 lightPos; // 4th component stores the distance
			SceneFlags flags;
			float alphaCutoff;
			uint32_t vertexAnimInfo;
		};
#pragma pack(pop)

		ShaderShadow(prosper::IPrContext &context, const std::string &identifier);
		ShaderShadow(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader);

		virtual GameShaderType GetPassType() const { return GameShaderType::ShadowPass; }

		//
		virtual void RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
		  prosper::IDescriptorSet &dsLights, prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, ShaderGameWorld::SceneFlags &inOutSceneFlags) const override;
		virtual void RecordSceneFlags(rendering::ShaderProcessor &shaderProcessor, SceneFlags sceneFlags) const override;
		virtual void RecordBindLight(rendering::ShaderProcessor &shaderProcessor, CLightComponent &light, uint32_t layerId) const override;
		virtual void RecordAlphaCutoff(rendering::ShaderProcessor &shaderProcessor, float alphaCutoff) const override;
		virtual bool RecordBindMaterial(rendering::ShaderProcessor &shaderProcessor, CMaterial &mat) const override;
		virtual void RecordClipPlane(rendering::ShaderProcessor &shaderProcessor, const Vector4 &clipPlane) const override {}
		virtual void RecordDrawOrigin(rendering::ShaderProcessor &shaderProcessor, const Vector4 &drawOrigin) const {}
		virtual void RecordDepthBias(rendering::ShaderProcessor &shaderProcessor, const Vector2 &depthBias) const override {}
		virtual void RecordVertexAnimationOffset(rendering::ShaderProcessor &shaderProcessor, uint32_t vertexAnimationOffset) const override;
	  protected:
		bool BindEntityDepthMatrix(const Mat4 &depthMVP);
		virtual void OnPipelinesInitialized() override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual uint32_t GetSceneDescriptorSetIndex() const override;
	  private:
		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const override;
		virtual uint32_t GetCameraDescriptorSetIndex() const override;
		virtual uint32_t GetLightDescriptorSetIndex() const override;
		virtual uint32_t GetInstanceDescriptorSetIndex() const override;
		virtual void GetVertexAnimationPushConstantInfo(uint32_t &offset) const override;
	};

	class DLLCLIENT ShaderShadowTransparent : public ShaderShadow {
	  public:
		//bool BindMaterial(CMaterial &mat);
	};

	//////////////////

	class DLLCLIENT ShaderShadowSpot : public ShaderShadow {
	  public:
		ShaderShadowSpot(prosper::IPrContext &context, const std::string &identifier);
	};

	//////////////////

	class DLLCLIENT ShaderShadowCSM : public ShaderShadow {
	  public:
		ShaderShadowCSM(prosper::IPrContext &context, const std::string &identifier);
	  protected:
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
	};

	//////////////////

	class DLLCLIENT ShaderShadowCSMTransparent : public ShaderShadowCSM {
	  public:
		//bool BindMaterial(CMaterial &mat);
	};
};

#endif
