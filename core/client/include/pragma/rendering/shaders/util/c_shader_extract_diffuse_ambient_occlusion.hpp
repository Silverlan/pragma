#ifndef __C_SHADER_EXTRACT_DIFFUSE_AMBIENT_OCCLUSION_HPP__
#define __C_SHADER_EXTRACT_DIFFUSE_AMBIENT_OCCLUSION_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader_base_image_processing.hpp>

namespace prosper {class Texture;};
namespace pragma
{
	class DLLCLIENT ShaderExtractDiffuseAmbientOcclusion
		: public prosper::ShaderBaseImageProcessing
	{
	public:
		ShaderExtractDiffuseAmbientOcclusion(prosper::Context &context,const std::string &identifier);
	};
};

#endif
