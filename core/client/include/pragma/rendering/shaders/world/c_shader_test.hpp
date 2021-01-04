/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_TEST_HPP__
#define __C_SHADER_TEST_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

namespace pragma
{
	class DLLCLIENT ShaderTest
		: public ShaderTextured3DBase
	{
	public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_PBR;

		enum class MaterialBinding : uint32_t
		{
			MaterialSettings = umath::to_integral(ShaderTextured3DBase::MaterialBinding::MaterialSettings),
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

		enum class PBRBinding : uint32_t
		{
			IrradianceMap = 0u,
			PrefilterMap,
			BRDFMap,

			Count
		};

		ShaderTest(prosper::IPrContext &context,const std::string &identifier);
		ShaderTest(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader="");

		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
		virtual bool BindSceneCamera(pragma::CSceneComponent &scene,const rendering::RasterizationRenderer &renderer,bool bView) override;
		virtual bool BindScene(pragma::CSceneComponent &scene,rendering::RasterizationRenderer &renderer,bool bView) override;
		virtual bool BeginDraw(
			const std::shared_ptr<prosper::ICommandBuffer> &cmdBuffer,const Vector4 &clipPlane,const Vector4 &drawOrigin={0.f,0.f,0.f,1.f},ShaderGameWorldPipeline pipelineIdx=ShaderGameWorldPipeline::Regular,
			RecordFlags recordFlags=RecordFlags::RenderPassTargetAsViewportAndScissor
		) override;
		virtual bool Draw(CModelSubMesh &mesh,const std::optional<pragma::RenderMeshIndex> &meshIdx,prosper::IBuffer &renderBufferIndexBuffer,uint32_t instanceCount=1) override;
		void DrawTest(prosper::IBuffer &buf,prosper::IBuffer &ibuf,uint32_t count);
		virtual bool BindMaterial(CMaterial &mat) override;
		virtual bool BindEntity(CBaseEntity &ent) override;
		void SetForceNonIBLMode(bool b);
	protected:
		using ShaderTextured3DBase::Draw;
		virtual bool BindRenderFlags(RenderFlags flags) override {return true;}
		virtual void InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void UpdateRenderFlags(CModelSubMesh &mesh,RenderFlags &inOutFlags) override;
		virtual bool BindMaterialParameters(CMaterial &mat) override;
		virtual prosper::DescriptorSetInfo &GetMaterialDescriptorSetInfo() const override;
		virtual void InitializeGfxPipelineDescriptorSets(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat,const prosper::DescriptorSetInfo &descSetInfo);

		RenderFlags m_extRenderFlags = RenderFlags::None;
		bool m_bNonIBLMode = false;
		Mat4 m_testMvp;
	};
};

#endif
