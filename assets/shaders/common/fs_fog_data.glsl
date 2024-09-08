#ifndef F_FS_FOG_DATA_GLS
#define F_FS_FOG_DATA_GLS

#include "/functions/fs_linearize_depth.glsl"

struct Fog {
	vec4 color;
	float start;
	float end;
	float density;
	uint type;
	uint flags;
};

#define FOG_TYPE_LINEAR 0
#define FOG_TYPE_EXP 1
#define FOG_TYPE_EXP2 2

float get_fog_factor(Fog fog, float fogCoord)
{
	// See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glFog.xml
	float r;
	switch(fog.type) {
	case FOG_TYPE_EXP:
		r = exp(-fog.density * fogCoord);
		r = 1.0 - clamp(r, 0.0, 1.0);
		break;
	case FOG_TYPE_EXP2:
		r = exp(-pow(fog.density * fogCoord, 2.0));
		r = 1.0 - clamp(r, 0.0, 1.0);
		break;
	default:
		r = (fog.end - fogCoord) / (fog.end - fog.start);
		r = 1.0 - clamp(r, 0.0, 1.0);
		r = min(r, fog.density);
		break;
	}
	return r;
}
vec3 calculate_fog(Fog fog, vec3 color, float depth) { return mix(color, fog.color.rgb, get_fog_factor(fog, depth)).rgb; }
vec3 calculate_fog(Fog fog, vec3 color, float nearZ, float farZ) { return calculate_fog(fog, color, get_depth_distance(nearZ, farZ)); }

#endif
