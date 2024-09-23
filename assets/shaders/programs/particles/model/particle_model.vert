#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "sh_particle_model.gls"
#include "../sh_particle_render_flags.gls"
#include "../../modules/vs_world.gls"
#include "../../modules/sh_matrix.gls"
#include "../../modules/sh_color.gls"

#define SHADER_PARTICLE_XYZS_BUFFER_LOCATION (SHADER_NORMAL_BITANGENT_BUFFER_LOCATION + 1)
#define SHADER_PARTICLE_PREVPOS_BUFFER_LOCATION (SHADER_PARTICLE_XYZS_BUFFER_LOCATION + 1)
#define SHADER_PARTICLE_COLOR_BUFFER_LOCATION (SHADER_PARTICLE_PREVPOS_BUFFER_LOCATION + 1)
#define SHADER_PARTICLE_ROTATION_BUFFER_LOCATION (SHADER_PARTICLE_COLOR_BUFFER_LOCATION + 1)
#define SHADER_PARTICLE_ANIMATION_START_LOCATION (SHADER_PARTICLE_ROTATION_BUFFER_LOCATION + 1)

layout(location = SHADER_PARTICLE_XYZS_BUFFER_LOCATION) in vec4 in_xyzs;
layout(location = SHADER_PARTICLE_PREVPOS_BUFFER_LOCATION) in vec4 in_prevPos;
layout(location = SHADER_PARTICLE_COLOR_BUFFER_LOCATION) in vec4 in_color;
layout(location = SHADER_PARTICLE_ROTATION_BUFFER_LOCATION) in vec4 in_rot;

layout(location = SHADER_PARTICLE_ANIMATION_START_LOCATION) in float in_animationStart;

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
	mat3 matRot = get_rotation_matrix(in_rot);
	mat4 matTranslate = mat4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, in_xyzs.x, in_xyzs.y, in_xyzs.z, 1.0);
	mat4 matScale = mat4(in_xyzs.w, 0.0, 0.0, 0.0, 0.0, in_xyzs.w, 0.0, 0.0, 0.0, 0.0, in_xyzs.w, 0.0, 0.0, 0.0, 0.0, 1.0);
	mat4 mdlMatrix = matTranslate * matScale * mat4(matRot);

	export_world_fragment_data(mdlMatrix, in_vert_pos);

	vec4 col = in_color;
	rgba8_to_rgba16(col);
	vs_pt_out.color = col;
	if((u_material.renderFlags & PARTICLE_FRENDER_TEXTURE_SCROLLING) == 0)
		vs_pt_out.particle_start = in_animationStart;
	else {
		vs_pt_out.particle_start = 0;

		uint scrollOffset = floatBitsToUint(in_animationStart);
		vs_out.vert_uv += unpackHalf2x16(scrollOffset);
	}
}
