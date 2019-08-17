#include "stdafx_client.h"
#include "pragma/rendering/shaders/util/c_shader_specular_to_roughness.hpp"

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

#pragma optimize("",off)
ShaderSpecularToRoughness::ShaderSpecularToRoughness(prosper::Context &context,const std::string &identifier)
	: ShaderBaseImageProcessing{context,identifier,"util/fs_specular_to_roughness.gls"}
{}
#pragma optimize("",on)
