#ifndef F_SH_SHADOW_GLS
#define F_SH_SHADOW_GLS

#include "/common/export.glsl"
#include "/common/scene_draw_info.glsl"
#include "/common/vertex_data_locations.glsl"

struct VertexData {
	vec3 vert_pos_ws;  // Vertex Position in world space
	vec4 light_pos_ws; // Light Position in world space; 4th component stores the distance
};
layout(location = SHADER_VERTEX_DATA_LOCATION) EXPORT_VS VertexData
#ifdef GLS_FRAGMENT_SHADER
  fs_in
#else
  vs_out
#endif
  ;

layout(LAYOUT_PUSH_CONSTANTS()) uniform pushConstants
{
	mat4 depthMVP;
	vec4 lightPos; // 4th component stores the distance
	uint flags;
	float alphaCutoff;
	uint vertexAnimInfo;
}
u_pushConstants;

bool is_weighted_ext() { return (u_pushConstants.flags & SCENE_FLAG_BIT_USE_EXTENDED_VERTEX_WEIGHTS) != 0; }

#endif
