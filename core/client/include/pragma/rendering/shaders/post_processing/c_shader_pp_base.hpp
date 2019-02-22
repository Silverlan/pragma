#ifndef __C_SHADER_PP_BASE_HPP__
#define __C_SHADER_PP_BASE_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader_base_image_processing.hpp>

namespace pragma
{
	class DLLCLIENT ShaderPPBase
		: public prosper::ShaderBaseImageProcessing
	{
	public:
		ShaderPPBase(prosper::Context &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader);
		ShaderPPBase(prosper::Context &context,const std::string &identifier,const std::string &fsShader);
	protected:
		virtual void InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx) override;
	};
};

#endif
