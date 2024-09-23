#ifndef F_CS_WATER_SURFACE_EDGE_DATA_GLS
#define F_CS_WATER_SURFACE_EDGE_DATA_GLS

struct ParticleEdgeData {
	vec4 thisToNb;
	vec4 nbToThis;
};

layout(std140, LAYOUT_ID(WATER, EDGE_DATA)) buffer CS_WATER_EDGES { ParticleEdgeData edgeData[]; }
u_waterParticleEdgeData;

#endif
