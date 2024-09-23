#ifndef F_VS_INSTANCED_ENTITY_DATA_GLS
#define F_VS_INSTANCED_ENTITY_DATA_GLS

#ifndef ENABLE_INSTANCED_ENTITIES
#define ENABLE_INSTANCED_ENTITIES 1
#endif

#include "/common/inputs/entity.glsl"
#include "/common/vertex_outputs/vs_vertex_data_locations.glsl"
#include "/math/math.glsl"

#if ENABLE_INSTANCED_ENTITIES == 1
#include "/common/inputs/entity_instance_data.glsl"
layout(location = SHADER_RENDER_BUFFER_INDEX_BUFFER_LOCATION) in uint in_render_buffer_index;
#endif

#define SINGLE_INSTANCE_RENDER_BUFFER_INDEX MAX_UINT
EntityInstanceData get_instance_data()
{
#if ENABLE_INSTANCED_ENTITIES == 1
	if(in_render_buffer_index != SINGLE_INSTANCE_RENDER_BUFFER_INDEX)
		return u_instances.data[in_render_buffer_index].entData;
#endif
	return u_instance.data;
	// This causes a VUID-VkPipelineShaderStageCreateInfo-module-04145 validation error
	// return (in_render_buffer_index != SINGLE_INSTANCE_RENDER_BUFFER_INDEX) ? u_instances.data[in_render_buffer_index].entData : u_instance.data;
}
mat4 get_instanced_model_matrix() { return get_instance_data().M; }
vec4 get_instanced_instance_color() { return get_instance_data().color; }
bool is_instanced_weighted() { return (get_instance_data().flags & FINSTANCE_WEIGHTED) != 0; }

#endif
