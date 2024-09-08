#ifndef F_SH_GLOBAL_ENTITY_INSTANCE_BUFFER_GLS
#define F_SH_GLOBAL_ENTITY_INSTANCE_BUFFER_GLS

#include "/common/entity_data.glsl"

struct StorageEntityInstanceData {
	EntityInstanceData entData;
	vec4 padding[10]; // Padding to 256 bytes (uniform buffer alignment)
};

// Storage buffer containing instance data for *all* entities; can be indexed
layout(std140, LAYOUT_ID(RENDER_SETTINGS, GLOBAL_ENTITY_INSTANCE_DATA)) buffer Instances { StorageEntityInstanceData data[]; }
u_instances;

#endif
