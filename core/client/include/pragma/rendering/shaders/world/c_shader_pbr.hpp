#ifndef __C_SHADER_PBR_HPP__
#define __C_SHADER_PBR_HPP__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

namespace pragma
{
	class DLLCLIENT ShaderPBR
		: public ShaderTextured3DBase
	{
	public:
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_PBR;

		enum class MaterialBinding : uint32_t
		{
			MaterialSettings = umath::to_integral(ShaderTextured3DBase::MaterialBinding::MaterialSettings),
			AlbedoMap,
			NormalMap,
			AmbientOcclusionMap,
			MetallicMap,
			RoughnessMap,
			EmissionMap,
			ParallaxMap,
			WrinkleStretchMap,
			WrinkleCompressMap,
			ExponentMap,
			SpecularMap,

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

		virtual std::shared_ptr<prosper::DescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
		virtual bool BindSceneCamera(const rendering::RasterizationRenderer &renderer,bool bView) override;
		virtual bool BeginDraw(
			const std::shared_ptr<prosper::PrimaryCommandBuffer> &cmdBuffer,const Vector4 &clipPlane,Pipeline pipelineIdx=Pipeline::Regular,
			RecordFlags recordFlags=RecordFlags::RenderPassTargetAsViewportAndScissor
		) override;
		void SetForceNonIBLMode(bool b);
	protected:
		using ShaderEntity::Draw;
		virtual void UpdateRenderFlags(CModelSubMesh &mesh,RenderFlags &inOutFlags) override;
		virtual bool BindMaterialParameters(CMaterial &mat) override;
		virtual prosper::Shader::DescriptorSetInfo &GetMaterialDescriptorSetInfo() const override;
		virtual void InitializeGfxPipelineDescriptorSets(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		std::shared_ptr<prosper::DescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat,const prosper::Shader::DescriptorSetInfo &descSetInfo);

		RenderFlags m_extRenderFlags = RenderFlags::None;
		bool m_bNonIBLMode = false;
	};
};

#endif
