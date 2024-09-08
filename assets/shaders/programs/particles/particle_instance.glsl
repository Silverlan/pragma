#ifndef F_SH_PARTICLE_INSTANCE_GLS
#define F_SH_PARTICLE_INSTANCE_GLS

#ifndef PUSH_USER_CONSTANTS
#define PUSH_USER_CONSTANTS
#endif

#include "particle_render_flags.glsl"

layout(LAYOUT_PUSH_CONSTANTS()) uniform Instance
{
	vec4 colorFactor;
	vec3 camRight_ws;
	int orientation;
	vec3 camUp_ws;
	float nearZ;
	vec3 camPos;
	float farZ;
	uint viewportSize; // First 16 bits = width, second 16 bits = height
	uint renderFlags;
	uint alphaMode;
	float simTime;
	PUSH_USER_CONSTANTS
}
u_instance;

#endif
