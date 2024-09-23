#ifndef F_SH_ENTITY_INSTANCE_DATA_GLS
#define F_SH_ENTITY_INSTANCE_DATA_GLS

struct EntityInstanceData {
	mat4 M;
	vec4 color;
	uint flags;
	uint entityIndex;
	ivec2 padding;
};

#endif
