#ifndef F_FS_SPECULARMAPPING_GLS
#define F_FS_SPECULARMAPPING_GLS

#include "sh_uniform_locations.gls"
#include "sh_rendersettings.gls"

/*layout(std140,LAYOUT_ID(SHADER_UNIFORM_SPECULAR_SET,0)) uniform Specular
{
	float phongScale;
} u_specular;*/ // Defined in push constants
#ifndef USE_SPECULAR_MAP
#define USE_SPECULAR_MAP 1
#define ENABLE_PHONG 1
#endif
#ifndef ENABLE_PHONG
#define ENABLE_PHONG 0
#endif

#if USE_SPECULAR_MAP == 1
layout(LAYOUT_ID(MATERIAL, SPECULAR_MAP)) uniform sampler2D u_specularMap;
#endif
//uniform bool u_useSpecularMap; // Defined in push constants

#if ENABLE_PHONG == 1
#ifndef FC_USE_PHONG
#define FC_USE_PHONG
bool use_phong() { return false; } // OBSOLETE
#endif

#ifndef FC_GET_SPECULAR_COLOR
#define FC_GET_SPECULAR_COLOR
vec4 get_specular_color(vec2 uv) // OBSOLETE!
{
	return vec4(0, 0, 0, 1);
}
#endif
#endif

vec3 apply_phong(mat4 mdlMatrix, vec3 fragPosWs, vec3 fragNormal, vec3 incidenceVector, vec4 specularColor, vec3 lightColor)
{
	return vec3(0, 0, 0); // OBSOLETE!
}

vec3 calc_phong(mat4 mdlMatrix, vec3 fragPosWs, vec3 fragNormal, vec3 incidenceVector, vec4 specularColor, vec3 lightColor)
{
	return vec3(0, 0, 0); // OBSOLETE!
}

vec3 calc_phong(mat4 mdlMatrix, vec3 fragPosWs, vec3 fragNormal, vec3 incidenceVector, vec3 lightColor, vec2 uv)
{
	return vec3(0, 0, 0); // OBSOLETE!
}

#endif
