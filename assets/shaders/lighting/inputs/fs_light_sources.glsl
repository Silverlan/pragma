#ifndef F_SH_LIGHT_DATA_GLS
#define F_SH_LIGHT_DATA_GLS

#include "light_sources.glsl"
#include "/common/inputs/camera.glsl"

#ifndef LIGHTING_ENABLED
#define LIGHTING_ENABLED 1
#endif

#if LIGHTING_ENABLED == 1

#ifndef USE_LIGHT_DIR_TS
#define USE_LIGHT_DIR_TS 1
#endif

#ifdef GLS_FRAGMENT_SHADER
vec3 get_light_direction_cs(uint i)
{
	LightSourceData light = get_light_source(i);
	if((light.flags & FLIGHT_TYPE_DIRECTIONAL) != 0)
		return -(get_view_matrix() * vec4(light.direction.xyz, 0.0)).xyz;

	// TODO: No reason to do this for every light source individually
	mat4 vm = get_view_matrix();
	vm[3][0] = 0.0;
	vm[3][1] = 0.0;
	vm[3][2] = 0.0;
	return (vm * vec4(light.position.xyz - get_vertex_position_ws(), 1.0)).xyz;
}

vec4 get_light_vertex_position(uint i)
{
	LightSourceData light = get_light_source(i);
	ShadowData shadow = shadowBuffer.data[light.shadowIndex - 1];

	vec4 lightVertPos = shadow.depthVP * vec4(get_vertex_position_ws(), 1.0);
	lightVertPos.z /= light.position.w;
	return lightVertPos;
}
#endif

#if USE_LIGHT_DIR_TS == 1
vec3 get_light_direction_ts(uint i) { return get_light_direction_cs(i); }
#endif

#endif

#endif
