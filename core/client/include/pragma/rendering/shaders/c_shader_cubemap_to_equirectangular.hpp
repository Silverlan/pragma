#ifndef __C_SHADER_CUBEMAP_TO_EQUIRECTANGULAR_HPP__
#define __C_SHADER_CUBEMAP_TO_EQUIRECTANGULAR_HPP__

#include "pragma/rendering/shaders/c_shader_base_cubemap.hpp"
#include <shader/prosper_shader_base_image_processing.hpp>

namespace prosper {class Texture;};
namespace pragma
{
	class DLLCLIENT ShaderCubemapToEquirectangular
		: public prosper::ShaderBaseImageProcessing
	{
	public:
		ShaderCubemapToEquirectangular(prosper::Context &context,const std::string &identifier);
		std::shared_ptr<prosper::Texture> CubemapToEquirectangularTexture(prosper::Texture &cubemap,uint32_t width=1'600,uint32_t height=800);
	protected:
		std::shared_ptr<prosper::Image> CreateEquirectangularMap(uint32_t width,uint32_t height,prosper::util::ImageCreateInfo::Flags flags) const;
		std::shared_ptr<prosper::RenderTarget> CreateEquirectangularRenderTarget(uint32_t width,uint32_t height,prosper::util::ImageCreateInfo::Flags flags) const;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx) override;
	};
};

#endif
