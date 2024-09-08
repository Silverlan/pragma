#ifndef F_EQUIRECTANGULAR_GLS
#define F_EQUIRECTANGULAR_GLS

// Source: https://learnopengl.com
vec2 direction_to_equirectangular_uv_coordinates(vec3 v, float xfactor)
{
	const vec2 invAtan = vec2(0.1591, 0.3183);
	vec2 uv = vec2(atan(v.x, v.z), asin(v.y));
	uv *= invAtan;
	uv.x *= xfactor;
	uv += 0.5;
	return vec2(uv.x, 1.0 - uv.y);
}

vec2 direction_to_equirectangular_uv_coordinates(vec3 v) { return direction_to_equirectangular_uv_coordinates(v, 1.0); }

#endif
