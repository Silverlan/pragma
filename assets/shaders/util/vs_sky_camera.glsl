#ifndef F_VS_SKY_CAMERA_GLS
#define F_VS_SKY_CAMERA_GLS

void apply_sky_camera_transform(vec4 skyTransform, inout mat4 mdlMatrix)
{
	float s = skyTransform.w;
	mat4 ms = mat4(s, 0, 0, 0, 0, s, 0, 0, 0, 0, s, 0, 0, 0, 0, 1);
	vec3 t = -skyTransform.xyz;
	mdlMatrix[3][0] += t.x;
	mdlMatrix[3][1] += t.y;
	mdlMatrix[3][2] += t.z;
	mdlMatrix = ms * mdlMatrix;
}

#endif
