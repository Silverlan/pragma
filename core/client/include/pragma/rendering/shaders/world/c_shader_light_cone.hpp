#ifndef __C_SHADER_LIGHT_CONE_HPP__
#define __C_SHADER_LIGHT_CONE_HPP__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

namespace pragma
{
	namespace rendering {class RasterizationRenderer;};
	class DLLCLIENT ShaderLightCone
		: public ShaderTextured3DBase
	{
	public:
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_DEPTH_MAP;

#pragma pack(push,1)
		struct PushConstants
		{
			float coneLength;
			uint32_t boundLightIndex;
		};
#pragma pack(pop)

		ShaderLightCone(prosper::Context &context,const std::string &identifier);
		virtual bool BindSceneCamera(const pragma::rendering::RasterizationRenderer &renderer,bool bView) override;
		virtual bool BindEntity(CBaseEntity &ent) override;
		virtual bool Draw(CModelSubMesh &mesh) override;
	protected:
		virtual bool BindMaterialParameters(CMaterial &mat) override;
		virtual void InitializeGfxPipelinePushConstantRanges(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		int32_t m_boundLightIndex = -1;
	};
};

#endif
