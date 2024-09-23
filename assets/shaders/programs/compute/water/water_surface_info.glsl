#ifndef F_CS_WATER_SURFACE_INFO_GLS
#define F_CS_WATER_SURFACE_INFO_GLS

layout(std140, LAYOUT_ID(SURFACE, INFO)) uniform CS_SURFACE_INFO
{
	vec3 origin;
	float stiffness;
	float propagation;
	float maxHeight;
	uint spacing;
	uint width;
	uint height; // Actually 'length', but 'length' is a reserved name
}
u_surfaceInfo;

#endif
