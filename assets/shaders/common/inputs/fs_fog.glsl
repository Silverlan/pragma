#ifndef F_FS_FOG_GLS
#define F_FS_FOG_GLS

#include "/common/fog.glsl"
#include "/functions/fs_linearize_depth.glsl"

layout(std140, LAYOUT_ID(FOG, DATA)) uniform UFog { Fog fog; }
u_fog;
//bool is_fog_enabled() {return (u_fog.fog.flags &1) != 0;}

vec3 get_fog_color() { return u_fog.fog.color.rgb; }

float get_fog_factor(float fogCoord)
{
	Fog f = u_fog.fog;
	return get_fog_factor(f, fogCoord);
}

vec3 calculate_fog(vec3 color, float depth) { return mix(color, u_fog.fog.color.rgb, get_fog_factor(depth)).rgb; }
vec3 calculate_fog(vec3 color, float nearZ, float farZ) { return calculate_fog(color, get_depth_distance(nearZ, farZ)); }

#endif
