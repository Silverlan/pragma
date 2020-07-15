/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_SKYBOX_H__
#define __C_SHADER_SKYBOX_H__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

namespace pragma
{
	class DLLCLIENT ShaderSkybox
		: public ShaderTextured3DBase
	{
	public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_POSITION;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_INSTANCE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_CAMERA;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;

#pragma pack(push,1)
		struct PushConstants
		{
			Vector3 posCam;
		};
#pragma pack(pop)

		ShaderSkybox(prosper::IPrContext &context,const std::string &identifier);
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
		virtual bool BeginDraw(
			const std::shared_ptr<prosper::IPrimaryCommandBuffer> &cmdBuffer,const Vector4 &clipPlane,const Vector4 &drawOrigin={0.f,0.f,0.f,1.f},Pipeline pipelineIdx=Pipeline::Regular,
			RecordFlags recordFlags=RecordFlags::RenderPassTargetAsViewportAndScissor
		) override;
		virtual bool BindEntity(CBaseEntity &ent) override;
		virtual bool BindRenderSettings(prosper::IDescriptorSet &descSetRenderSettings) override;
		virtual bool BindSceneCamera(const pragma::rendering::RasterizationRenderer &renderer,bool bView) override;
		virtual bool BindLights(prosper::IDescriptorSet &dsLights) override;
		virtual bool BindVertexAnimationOffset(uint32_t offset) override;
		virtual bool Draw(CModelSubMesh &mesh) override;
	protected:
		ShaderSkybox(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader);
		virtual uint32_t GetMaterialDescriptorSetIndex() const override;
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
