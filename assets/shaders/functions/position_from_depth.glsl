#ifndef F_SH_POSITION_FROM_DEPTH_GLS
#define F_SH_POSITION_FROM_DEPTH_GLS

vec3 calc_position_from_depth(in float depth, in vec2 uv, in mat4 invVP)
{
	vec4 screenSpacePosition = vec4(uv * 2.0 - 1.0, depth, 1.0);
	vec4 worldSpacePosition = invVP * screenSpacePosition;
	return worldSpacePosition.xyz / worldSpacePosition.w;
}

#endif
