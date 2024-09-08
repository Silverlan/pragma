#ifndef F_SH_LIGHT_CONE_GLS
#define F_SH_LIGHT_CONE_GLS

#define SHADER_RENDER_BUFFER_INDEX_BUFFER_LOCATION 0
#define SHADER_VERTEX_BUFFER_LOCATION (SHADER_RENDER_BUFFER_INDEX_BUFFER_LOCATION + 1)

#define DESCRIPTOR_SET_DEPTH_MAP 1
#define DESCRIPTOR_SET_DEPTH_MAP_BINDING_MAP 0

#define DESCRIPTOR_SET_INSTANCE 2
#define DESCRIPTOR_SET_INSTANCE_BINDING_INSTANCE 0

#define DESCRIPTOR_SET_SCENE 3
#define DESCRIPTOR_SET_SCENE_BINDING_CAMERA 0
#define DESCRIPTOR_SET_SCENE_BINDING_RENDER_SETTINGS 1

#include "/common/export.glsl"

layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants
{
	vec4 color;
	vec4 coneOrigin; // w = coneLength
	uint resolution;
	float nearZ;
	float farZ;
}
u_pushConstants;

struct VertexData {
	vec3 vert_pos_ws;    // Vertex Position in world space
	vec3 vert_normal_cs; // Vertex Normal in camera space
};
layout(location = 0) EXPORT_VS VertexData
#ifdef GLS_FRAGMENT_SHADER
  fs_in
#else
  vs_out
#endif
  ;

#endif
