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

namespace pragma
{
	class DLLCLIENT ShaderTest
		: public ShaderGameWorldLightingPass
	{
	public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_PBR;

		enum class MaterialBinding : uint32_t
		{
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
		virtual bool BindSceneCamera(pragma::CSceneComponent &scene,const CRasterizationRendererComponent &renderer,bool bView) override;
		virtual bool BindScene(pragma::CSceneComponent &scene,CRasterizationRendererComponent &renderer,bool bView) override;
		virtual bool BeginDraw(
			const std::shared_ptr<prosper::ICommandBuffer> &cmdBuffer,const Vector4 &clipPlane,const Vector4 &drawOrigin={0.f,0.f,0.f,1.f},
			RecordFlags recordFlags=RecordFlags::RenderPassTargetAsViewportAndScissor
		) override;
		virtual bool Draw(CModelSubMesh &mesh,const std::optional<pragma::RenderMeshIndex> &meshIdx,prosper::IBuffer &renderBufferIndexBuffer,uint32_t instanceCount=1) override;
		void DrawTest(prosper::IBuffer &buf,prosper::IBuffer &ibuf,uint32_t count);
		virtual bool BindMaterial(CMaterial &mat) override;
		virtual bool BindEntity(CBaseEntity &ent) override;
		void SetForceNonIBLMode(bool b);
	protected:
		using ShaderGameWorldLightingPass::Draw;
		virtual prosper::DescriptorSetInfo &GetMaterialDescriptorSetInfo() const override;
		virtual bool BindRenderFlags(SceneFlags flags) override {return true;}
		virtual void InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void UpdateRenderFlags(CModelSubMesh &mesh,SceneFlags &inOutFlags) override;
		virtual bool BindMaterialParameters(CMaterial &mat) override;
		virtual void InitializeGfxPipelineDescriptorSets(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat,const prosper::DescriptorSetInfo &descSetInfo);

		SceneFlags m_extRenderFlags = SceneFlags::None;
		bool m_bNonIBLMode = false;
		Mat4 m_testMvp;
	};
};

#endif
