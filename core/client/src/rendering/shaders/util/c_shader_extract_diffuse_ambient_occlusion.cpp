#include "stdafx_client.h"
#include "pragma/rendering/shaders/util/c_shader_extract_diffuse_ambient_occlusion.hpp"

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;


ShaderExtractDiffuseAmbientOcclusion::ShaderExtractDiffuseAmbientOcclusion(prosper::Context &context,const std::string &identifier)
	: ShaderBaseImageProcessing{context,identifier,"util/fs_extract_diffuse_ambient_occlusion.gls"}
{}

