/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_PREPASS_HPP__
#define __C_SHADER_PREPASS_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include <shader/prosper_shader.hpp>

namespace pragma
{
	class DLLCLIENT ShaderPrepassBase
		: public ShaderGameWorld
	{
	public:
		enum class MaterialBinding : uint32_t
		{
			AlbedoMap = 0u,

			Count
		};
		static prosper::util::RenderPassCreateInfo::AttachmentInfo get_depth_render_pass_attachment_info(prosper::SampleCountFlags sampleCount);

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
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_UV;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_INSTANCE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDER_SETTINGS;

#pragma pack(push,1)
		struct PushConstants
			: public ScenePushConstants
		{
			float alphaCutoff;

			void Initialize()
			{
				ScenePushConstants::Initialize();
				alphaCutoff = 0.5f;
			}
		};
#pragma pack(pop)

		ShaderPrepassBase(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader);
		ShaderPrepassBase(prosper::IPrContext &context,const std::string &identifier);

		virtual bool BeginDraw(
			const std::shared_ptr<prosper::ICommandBuffer> &cmdBuffer,const Vector4 &clipPlane,const Vector4 &drawOrigin={0.f,0.f,0.f,1.f},
			RecordFlags recordFlags=RecordFlags::RenderPassTargetAsViewportAndScissor
		) override;
		virtual bool BindClipPlane(const Vector4 &clipPlane) override;
		virtual bool BindScene(pragma::CSceneComponent &scene,CRasterizationRendererComponent &renderer,bool bView) override;
		virtual bool BindDrawOrigin(const Vector4 &drawOrigin) override;
		virtual bool SetDepthBias(const Vector2 &depthBias) override;
		virtual void Set3DSky(bool is3dSky) override;
		virtual bool Draw(CModelSubMesh &mesh,const std::optional<pragma::RenderMeshIndex> &meshIdx,prosper::IBuffer &renderBufferIndexBuffer,uint32_t instanceCount=1) override;
		virtual GameShaderType GetPassType() const {return GameShaderType::DepthPrepass;}

		//
		virtual void RecordBindScene(
			rendering::ShaderProcessor &shaderProcessor,
			const pragma::CSceneComponent &scene,const pragma::CRasterizationRendererComponent &renderer,
			prosper::IDescriptorSet &dsScene,prosper::IDescriptorSet &dsRenderer,
			prosper::IDescriptorSet &dsRenderSettings,prosper::IDescriptorSet &dsLights,
			prosper::IDescriptorSet &dsShadows,prosper::IDescriptorSet &dsMaterial,
			ShaderGameWorld::SceneFlags &inOutSceneFlags
		) const override;
		virtual void RecordAlphaCutoff(rendering::ShaderProcessor &shaderProcessor,float alphaCutoff) const override;
		virtual bool RecordBindMaterial(rendering::ShaderProcessor &shaderProcessor,CMaterial &mat) const override;
	protected:
		virtual void OnPipelinesInitialized() override;
		virtual bool BindMaterial(CMaterial &mat) override;
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		
		virtual uint32_t GetMaterialDescriptorSetIndex() const override;
		virtual uint32_t GetCameraDescriptorSetIndex() const override;
		virtual uint32_t GetInstanceDescriptorSetIndex() const override;
		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const override;
		virtual void GetVertexAnimationPushConstantInfo(uint32_t &offset) const override;
	private:
		// These are unused
		virtual uint32_t GetLightDescriptorSetIndex() const {return std::numeric_limits<uint32_t>::max();}
		virtual bool BindLights(prosper::IDescriptorSet &dsLights) override {return false;}
		std::shared_ptr<prosper::IDescriptorSetGroup> m_dummyMaterialDsg = nullptr;
		std::optional<float> m_alphaCutoff {};
	};

	//////////////////

	class DLLCLIENT ShaderPrepass
		: public ShaderPrepassBase
	{
	public:
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_NORMAL;

		static prosper::Format RENDER_PASS_NORMAL_FORMAT;
		static prosper::util::RenderPassCreateInfo::AttachmentInfo get_normal_render_pass_attachment_info(prosper::SampleCountFlags sampleCount);

		enum class Pipeline : uint32_t
		{
			Opaque = 0u,
			AlphaTest,
			AnimatedOpaque,
			AnimatedAlphaTest,

			Count
		};

		enum class SpecializationConstant : uint32_t
		{
			EnableAlphaTest = 0u,
			EnableNormalOutput,
			EnableAnimation,
			EnableMorphTargetAnimation,
			EnableExtendedVertexWeights,

			Count
		};

		ShaderPrepass(prosper::IPrContext &context,const std::string &identifier);
	protected:
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

#endif