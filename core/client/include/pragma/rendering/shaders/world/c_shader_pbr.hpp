/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_PBR_HPP__
#define __C_SHADER_PBR_HPP__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

namespace pragma
{
	class DLLCLIENT ShaderPBR
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

		ShaderPBR(prosper::Context &context,const std::string &identifier);
		ShaderPBR(prosper::Context &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader="");

		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
		virtual bool BindSceneCamera(const rendering::RasterizationRenderer &renderer,bool bView) override;
		virtual bool BeginDraw(
			const std::shared_ptr<prosper::IPrimaryCommandBuffer> &cmdBuffer,const Vector4 &clipPlane,const Vector4 &drawOrigin={0.f,0.f,0.f,1.f},Pipeline pipelineIdx=Pipeline::Regular,
			RecordFlags recordFlags=RecordFlags::RenderPassTargetAsViewportAndScissor
		) override;
		void SetForceNonIBLMode(bool b);
	protected:
		using ShaderTextured3DBase::Draw;
		virtual void UpdateRenderFlags(CModelSubMesh &mesh,RenderFlags &inOutFlags) override;
		virtual bool BindMaterialParameters(CMaterial &mat) override;
		virtual prosper::DescriptorSetInfo &GetMaterialDescriptorSetInfo() const override;
		virtual void InitializeGfxPipelineDescriptorSets(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat,const prosper::DescriptorSetInfo &descSetInfo);

		RenderFlags m_extRenderFlags = RenderFlags::None;
		bool m_bNonIBLMode = false;
	};

	class DLLCLIENT ShaderPBRBlend
		: public ShaderPBR
	{
	public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_ALPHA;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_ALPHA;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;
		enum class MaterialBinding : uint32_t
		{
			AlbedoMap2 = umath::to_integral(ShaderPBR::MaterialBinding::Count),
			AlbedoMap3
		};

#pragma pack(push,1)
		struct PushConstants
		{
			int32_t alphaCount;
		};
#pragma pack(pop)

		ShaderPBRBlend(prosper::Context &context,const std::string &identifier);
		virtual bool Draw(CModelSubMesh &mesh) override;
	protected:
		virtual void InitializeGfxPipelinePushConstantRanges(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipelineVertexAttributes(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx);
		virtual prosper::DescriptorSetInfo &GetMaterialDescriptorSetInfo() const override;
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
	};
};

#endif
