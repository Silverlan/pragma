#ifndef F_FS_SSAO_GLS
#define F_FS_SSAO_GLS

#include "/common/inputs/textures/ssao_map.glsl"

#if ENABLE_SSAO == 1
float get_ssao_occlusion(uint vpWidth, uint vpHeight)
{
	vec2 uv = gl_FragCoord.xy;
	uv.x /= float(vpWidth);
	uv.y /= float(vpHeight);
	return texture(u_ssao, uv).r;
}
#endif

#endif
