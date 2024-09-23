#ifndef F_CS_WATER_PARTICLE_INSTANCE_GLS
#define F_CS_WATER_PARTICLE_INSTANCE_GLS

struct Vertex {
	vec4 position;
};

layout(std140, LAYOUT_ID(WATER, POSITIONS)) buffer CS_VERTEX_DATA { Vertex vertices[]; }
u_vertices;

#endif
