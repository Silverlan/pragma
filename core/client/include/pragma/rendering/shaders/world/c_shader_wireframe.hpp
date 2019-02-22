#ifndef __C_SHADER_WIREFRAME_HPP__
#define __C_SHADER_WIREFRAME_HPP__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

namespace pragma
{
	class DLLCLIENT ShaderWireframe
		: public ShaderTextured3D
	{
	public:
		ShaderWireframe(prosper::Context &context,const std::string &identifier);
	protected:
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

#endif
