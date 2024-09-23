#ifndef F_SH_CLIP_PLANE_GLS
#define F_SH_CLIP_PLANE_GLS

#include "/math/math.glsl"

float get_clip_plane_distance(vec4 clipPlane, vec3 worldPos)
{
	if(length_sqr(clipPlane.xyz) > 0.0)
		return dot(vec4(-worldPos.xyz, 1.0), clipPlane);
	return 1.0;
}
void apply_clip_plane(vec4 clipPlane, vec3 worldPos) { gl_ClipDistance[0] = get_clip_plane_distance(clipPlane, worldPos); }

#endif
