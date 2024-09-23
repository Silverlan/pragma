#ifndef F_VS_SHADOW_CSM_SHARED_GLS
#define F_VS_SHADOW_CSM_SHARED_GLS

#define SHADER_RENDER_BUFFER_INDEX_BUFFER_LOCATION 0

#define SHADER_BONE_WEIGHT_ID_LOCATION (SHADER_RENDER_BUFFER_INDEX_BUFFER_LOCATION + 1)
#define SHADER_BONE_WEIGHT_LOCATION (SHADER_BONE_WEIGHT_ID_LOCATION + 1)

#define SHADER_BONE_WEIGHT_EXT_ID_LOCATION (SHADER_BONE_WEIGHT_LOCATION + 1)
#define SHADER_BONE_WEIGHT_EXT_LOCATION (SHADER_BONE_WEIGHT_EXT_ID_LOCATION + 1)

#define SHADER_VERTEX_BUFFER_LOCATION (SHADER_BONE_WEIGHT_EXT_LOCATION + 1)

#ifdef SHADOW_CSM_ENABLE_UV
#define SHADER_UV_BUFFER_LOCATION 3
#endif

#include "/common/inputs/entity.glsl"
#include "/common/inputs/vs_skeletal_animation.glsl"

layout(location = SHADER_VERTEX_BUFFER_LOCATION) in vec3 u_vert_pos;
#ifdef SHADOW_CSM_ENABLE_UV
layout(location = SHADER_UV_BUFFER_LOCATION) in vec2 u_vert_uv;
#endif

layout(LAYOUT_PUSH_CONSTANTS()) uniform pushConstants { mat4 MVP; }
u_depth;

void main()
{
	vec4 vpos = get_weighted_vertex_position(is_weighted(), vec4(u_vert_pos, 1.0));
	gl_Position = u_depth.MVP * vpos;
#ifdef SHADOW_CSM_ENABLE_UV
	vs_out.vert_uv = u_vert_uv;
#endif
}

#endif
