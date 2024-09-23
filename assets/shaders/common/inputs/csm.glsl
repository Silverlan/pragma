#ifndef F_SH_CSM_GLS
#define F_SH_CSM_GLS

#include "/common/limits.glsl"
#include "/common/conditionals.glsl"

layout(std140, LAYOUT_ID(RENDER_SETTINGS, CSM_DATA)) uniform CSM
{
	mat4 VP[4]; // View-Projection Matrix
	vec4 fard;
	int count;
}
u_csm;
#ifdef GLS_FRAGMENT_SHADER
layout(LAYOUT_ID(LIGHTS, CSM_MAPS)) uniform sampler2D csmTextures[MAX_CSM_CASCADES];

int get_csm_cascade_index()
{
	// Select CSM index from distance
	return min(0 + int(when_gt(gl_FragCoord.z, u_csm.fard[0])) + int(when_gt(gl_FragCoord.z, u_csm.fard[1])) + int(when_gt(gl_FragCoord.z, u_csm.fard[2])), u_csm.count - 1);
}
#endif

#endif
