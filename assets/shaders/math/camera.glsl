#ifndef F_CAMERA_GLS
#define F_CAMERA_GLS

vec3 get_far_plane_center(in vec3 camOrigin, in vec3 forward, in float farZ) { return camOrigin + forward * farZ; }

vec2 get_far_plane_bounds(in float fov, in float farZ, in float aspectRatio)
{
	float y = (-(2.0 * tan(fov / 2.0) * farZ)) * 2.0;
	return vec2(y,
	  y //y *aspectRatio
	);
}

vec3 calc_far_plane_point(in vec3 camOrigin, in vec3 forward, in vec3 right, in vec3 up, in float fov, float farZ, float aspectRatio, in vec2 uv)
{
	vec3 center = get_far_plane_center(camOrigin, forward, farZ);
	vec2 planeBounds = get_far_plane_bounds(fov, farZ, aspectRatio);
	center += right * -(planeBounds.x / 2.0 * (uv.x - 0.5)) + up * (planeBounds.y / 2.0 * (uv.y - 0.5));
	return center;
}

#endif
