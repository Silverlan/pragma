#ifndef F_SH_CAMERA_INFO_GLS
#define F_SH_CAMERA_INFO_GLS

// Change per camera
layout(std140, LAYOUT_ID(SCENE, CAMERA)) uniform Camera
{
	mat4 V;
	mat4 P;
	mat4 VP;
}
u_camera;

mat4 get_view_matrix() { return u_camera.V; }
mat4 get_projection_matrix() { return API_COORD_TRANSFORM(u_camera.P); }
mat4 get_view_projection_matrix() { return API_COORD_TRANSFORM(u_camera.VP); }

#endif
