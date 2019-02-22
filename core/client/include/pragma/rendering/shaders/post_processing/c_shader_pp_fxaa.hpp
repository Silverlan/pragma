#ifndef __C_SHADER_PP_FXAA_HPP__
#define __C_SHADER_PP_FXAA_HPP__

#include "pragma/rendering/shaders/post_processing/c_shader_pp_base.hpp"

namespace pragma
{
	class DLLCLIENT ShaderPPFXAA
		: public ShaderPPBase
	{
	public:
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;

#pragma pack(push,1)
		struct DLLCLIENT PushConstants
		{
			float subPixelAliasingRemoval = 0.75f;
			float edgeThreshold = 0.166f;
			float minEdgeThreshold = 0.0833f;
		};
#pragma pack(pop)

		ShaderPPFXAA(prosper::Context &context,const std::string &identifier);
		bool Draw(Anvil::DescriptorSet &descSetTexture,const PushConstants &pushConstants={});
	protected:
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

#endif
