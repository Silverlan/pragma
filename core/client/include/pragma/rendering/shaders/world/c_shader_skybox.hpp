/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_SKYBOX_H__
#define __C_SHADER_SKYBOX_H__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

namespace pragma
{
	class DLLCLIENT ShaderSkybox
		: public ShaderGameWorldLightingPass
	{
	public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_POSITION;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_INSTANCE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDERER;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;

		ShaderSkybox(prosper::IPrContext &context,const std::string &identifier);
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
		virtual bool BeginDraw(
			const std::shared_ptr<prosper::ICommandBuffer> &cmdBuffer,const Vector4 &clipPlane,const Vector4 &drawOrigin={0.f,0.f,0.f,1.f},
			RecordFlags recordFlags=RecordFlags::RenderPassTargetAsViewportAndScissor
		) override;
		virtual bool BindClipPlane(const Vector4 &clipPlane) override {return true;}
		virtual bool BindEntity(CBaseEntity &ent) override;
		virtual bool BindRenderSettings(prosper::IDescriptorSet &descSetRenderSettings) override;
		virtual bool BindSceneCamera(pragma::CSceneComponent &scene,const pragma::CRasterizationRendererComponent &renderer,bool bView) override;
		virtual bool BindLights(prosper::IDescriptorSet &dsLights) override;
		virtual bool BindVertexAnimationOffset(uint32_t offset) override;
		virtual bool Draw(CModelSubMesh &mesh,const std::optional<pragma::RenderMeshIndex> &meshIdx,prosper::IBuffer &renderBufferIndexBuffer,uint32_t instanceCount=1) override;
		virtual bool GetRenderBufferTargets(
			CModelSubMesh &mesh,uint32_t pipelineIdx,std::vector<prosper::IBuffer*> &outBuffers,std::vector<prosper::DeviceSize> &outOffsets,
			std::optional<prosper::IndexBufferInfo> &outIndexBufferInfo
		) const override;
		virtual ShaderGameWorld::GameShaderType GetPassType() const {return ShaderGameWorld::GameShaderType::SkyPass;}

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
		virtual void RecordDepthBias(rendering::ShaderProcessor &shaderProcessor,const Vector2 &depthBias) const override {}
		virtual void RecordVertexAnimationOffset(rendering::ShaderProcessor &shaderProcessor,uint32_t vertexAnimationOffset) const override {}
	protected:
		ShaderSkybox(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader);
		virtual uint32_t GetMaterialDescriptorSetIndex() const override;
		virtual uint32_t GetRendererDescriptorSetIndex() const override;
		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const override;
		virtual uint32_t GetCameraDescriptorSetIndex() const override;
		virtual uint32_t GetLightDescriptorSetIndex() const override;
		virtual uint32_t GetInstanceDescriptorSetIndex() const override;
		virtual prosper::DescriptorSetInfo &GetMaterialDescriptorSetInfo() const override;
		virtual bool BindMaterialParameters(CMaterial &mat) override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		EulerAngles m_skyAngles = {};
	};

	//////////////

	class DLLCLIENT ShaderSkyboxEquirect
		: public ShaderSkybox
	{
	public:
		ShaderSkyboxEquirect(prosper::IPrContext &context,const std::string &identifier);
	};
};

#endif
