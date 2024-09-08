#ifndef F_SH_PARTICLE_GLS
#define F_SH_PARTICLE_GLS

#include "/math/math.glsl"
#ifndef ENABLE_PARTICLE_TEXTURE
#define ENABLE_PARTICLE_TEXTURE 1
#endif

#ifdef GLS_FRAGMENT_SHADER
#include "fs_particle_texture.glsl"
#endif

#ifdef GLS_VERTEX_SHADER
#include "vs_particle_vertex.glsl"
layout(location = SHADER_COLOR_BUFFER_LOCATION) in vec4 in_color;
#endif

#endif
