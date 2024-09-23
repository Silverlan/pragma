#ifndef F_SH_SKYBOX_GLS
#define F_SH_SKYBOX_GLS

#define PUSH_USER_CONSTANTS vec4 skyAngles;

#define SHADER_VERTEX_BUFFER_LOCATION 1
#define ENABLE_INSTANCED_ENTITIES 0

#include "/common/vertex_data_locations.glsl"
#include "/common/export.glsl"

struct VertexData {
	vec4 color;
	vec3 cubemap_dir;
};

layout(location = SHADER_VERTEX_DATA_LOCATION) EXPORT_VS VertexData
#ifdef GLS_FRAGMENT_SHADER
  fs_in
#else
  vs_out
#endif
  ;

#endif
