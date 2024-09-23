#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define ENABLE_PARTICLE_TEXTURE 0
#define ANIMATION_RENDER_FLAGS PARTICLE_FRENDER_ANIMATED

#include "sh_particle_model.gls"

#define SHADER_UNIFORM_TEXTURE_ANIMATION_SET SHADER_UNIFORM_USER_SET1
#define SHADER_UNIFORM_TEXTURE_ANIMATION_BINDING 0

#include "../fs_particle_texture.gls"
#include "../../debug/fs_debug_csm.gls"
#include "../../modules/fs_world.gls"
#include "../../modules/sh_csm.gls"
#include "../../modules/sh_parallaxmapping.gls"

struct ParticleData {
	vec4 color;
	float particle_start;
};
layout(location = SHADER_USER1_LOCATION) EXPORT_VS ParticleData
#ifdef GLS_FRAGMENT_SHADER
  fs_pt_in
#else
  vs_pt_out
#endif
  ;

void main()
{
	vec2 texCoords = apply_parallax(use_parallax_map(), get_particle_uv(fs_in.vert_uv, fs_pt_in.particle_start));
	fs_color = get_processed_color(get_texture_color(u_diffuseMap, texCoords), texCoords);
	fs_color *= fs_pt_in.color;
	// TODO: This doesn't belong here (and is already called in fs_world.gls)
	fs_color.a = fs_color.a * clamp(max(max(fs_color.r, fs_color.g), fs_color.b), 0, 1); // Black to alpha

	fs_color = debug_get_csm_cascade_color(fs_color);
	extract_bright_color(fs_color);
}