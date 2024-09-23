#ifndef F_SH_RENDERSETTINGS_GLS
#define F_SH_RENDERSETTINGS_GLS

#define FRENDER_UNLIT 1

layout(std140, LAYOUT_ID(SCENE, RENDER_SETTINGS)) uniform RenderSettings
{
	vec4 posCam; // w component is fov
	int flags;
	float shadowRatioX;
	float shadowRatioY;
	float nearZ;
	float farZ;
	int shaderQuality; // 1 = lowest, 10 = highest
}
u_renderSettings;

bool is_unlit() { return (u_renderSettings.flags & FRENDER_UNLIT) != 0; }
vec3 get_cam_pos() { return u_renderSettings.posCam.xyz; }
float get_fov() { return u_renderSettings.posCam.w; }

#endif
