#ifndef F_SH_ENTITY_INFO_GLS
#define F_SH_ENTITY_INFO_GLS

#include "/common/entity_data.glsl"
#include "/common/instance_flags.glsl"

layout(std140, LAYOUT_ID(INSTANCE, ENTITY_DATA)) uniform Instance { EntityInstanceData data; }
u_instance;

#ifdef GLS_VERTEX_SHADER
mat4 get_model_matrix() { return u_instance.data.M; }
vec4 get_instance_color() { return u_instance.data.color; }
#endif

bool is_weighted() { return (u_instance.data.flags & FINSTANCE_WEIGHTED) != 0; }
uint get_entity_index() { return u_instance.data.entityIndex; }

#endif