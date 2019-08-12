#ifndef __C_SHADER_EQUIRECTANGULAR_TO_CUBEMAP_HPP__
#define __C_SHADER_EQUIRECTANGULAR_TO_CUBEMAP_HPP__

#include "pragma/rendering/shaders/c_shader_base_cubemap.hpp"

namespace prosper {class Texture;};
namespace pragma
{
	class DLLCLIENT ShaderEquirectangularToCubemap
		: public ShaderCubemap
	{
	public:
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_EQUIRECTANGULAR_TEXTURE;

		ShaderEquirectangularToCubemap(prosper::Context &context,const std::string &identifier);
		std::shared_ptr<prosper::Texture> EquirectangularTextureToCubemap(prosper::Texture &equirectangularTexture,uint32_t resolution);
		std::shared_ptr<prosper::Texture> LoadEquirectangularImage(const std::string &fileName,uint32_t resolution);
		std::shared_ptr<prosper::Texture> LoadEquirectangularImage(VFilePtr f,uint32_t resolution);
	protected:
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx) override;
	};
};

#endif
