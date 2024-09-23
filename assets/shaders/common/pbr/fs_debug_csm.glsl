#ifndef F_DEBUG_CSM_GLS
#define F_DEBUG_CSM_GLS

#include "/common/inputs/csm.glsl"

const float debugCSMColorScale = 1.f;
vec4 debug_get_csm_cascade_color(vec4 col)
{
	// Note: Using a switch-case here messes with the LunaGLASS optimizer.
	// Since these are only evaluated in debug mode, the overhead from the if-branches
	// (if there is one) doesn't really matter
	if(u_csm.count == 4) {
		if(gl_FragCoord.z < u_csm.fard.x)
			col.b += debugCSMColorScale;
		else if(gl_FragCoord.z < u_csm.fard.y)
			col.g += debugCSMColorScale;
		else if(gl_FragCoord.z < u_csm.fard.z)
			col.r += debugCSMColorScale;
		else if(gl_FragCoord.z < u_csm.fard.w) {
			col.r += debugCSMColorScale * 0.5f;
			col.g += debugCSMColorScale * 0.5f;
		}
	}
	else if(u_csm.count == 3) {
		if(gl_FragCoord.z < u_csm.fard.x)
			col.b += debugCSMColorScale;
		else if(gl_FragCoord.z < u_csm.fard.y)
			col.g += debugCSMColorScale;
		else if(gl_FragCoord.z < u_csm.fard.z)
			col.r += debugCSMColorScale;
	}
	else if(u_csm.count == 2) {
		if(gl_FragCoord.z < u_csm.fard.x)
			col.b += debugCSMColorScale;
		else if(gl_FragCoord.z < u_csm.fard.y)
			col.g += debugCSMColorScale;
	}
	else {
		if(gl_FragCoord.z < u_csm.fard.x)
			col.b += debugCSMColorScale;
	}
	return col;
}

#endif
