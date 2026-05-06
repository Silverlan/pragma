#ifndef F_SH_RENDERSETTINGS_GLS
#define F_SH_RENDERSETTINGS_GLS

#define FRENDER_UNLIT 1

layout(std140, LAYOUT_ID(SCENE, RENDER_SETTINGS)) uniform RenderSettings
{
	int flags;
	float shadowRatioX;
	float shadowRatioY;
	float nearZ;
	float farZ;
	int shaderQuality; // 1 = lowest, 10 = highest
}
u_renderSettings;

bool is_unlit() { return (u_renderSettings.flags & FRENDER_UNLIT) != 0; }

#endif
