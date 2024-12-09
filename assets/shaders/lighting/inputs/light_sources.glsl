#ifndef F_SH_LIGHT_SOURCES_GLS
#define F_SH_LIGHT_SOURCES_GLS

#include "/common/limits.glsl"
#include "/common/light_source.glsl"
#include "/math/math.glsl"
#include "vis_light_buffer.glsl"

const int LIGHT_TYPE_DIRECTIONAL = 1;
const int LIGHT_TYPE_POINT = 2;
const int LIGHT_TYPE_SPOT = 3;

#define USE_LIGHT_SOURCE_UNIFORM_BUFFER 1

struct ShadowData {
	mat4 depthVP;
	mat4 view;
	mat4 projection;
};

#if USE_LIGHT_SOURCE_UNIFORM_BUFFER == 0

layout(std430, LAYOUT_ID(RENDERER, LIGHT_BUFFERS)) readonly buffer LightBuffer { LightSourceData data[]; }
lightBuffer;
layout(std430, LAYOUT_ID(RENDERER, SHADOW_BUFFERS)) readonly buffer ShadowBuffer { ShadowData data[]; }
shadowBuffer;

#else

layout(std140, LAYOUT_ID(RENDERER, LIGHT_BUFFERS)) uniform LightBuffer { LightSourceData data[MAX_SCENE_LIGHTS]; }
lightBuffer;
layout(std140, LAYOUT_ID(RENDERER, SHADOW_BUFFERS)) uniform ShadowBuffer { ShadowData data[MAX_ACTIVE_SHADOW_SOURCES]; }
shadowBuffer;

#endif

LightSourceData get_light_source(uint i) { return lightBuffer.data[i]; }

// See https://de45xmedrsdbp.cloudfront.net/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf ("Real Shading in Unreal Engine 4" - Lighting Model)
float calc_light_falloff(in float distance, in float radius)
{
	float falloff = 0;
	float fDistOverRadius = pow4(distance / radius);
	falloff = pow2(clamp(1.0 - fDistOverRadius, 0.0, 1.0));
	falloff /= pow2(units_to_meters(distance)) + 1.0;
	return falloff;
}

float calc_physical_light_falloff(in float distance) { return 1.0 / pow2(units_to_meters(distance)); }

float calc_solid_angle(float apexAngleCos) { return 2 * M_PI * (1.0 - apexAngleCos); }

float candela_to_lumens(float candela, float apexAngleCos) { return candela * calc_solid_angle(apexAngleCos); }

float lumens_to_candela(float lumens, float apexAngleCos) { return lumens / calc_solid_angle(apexAngleCos); }

float lux_to_candela(float lux, float distance) { return lux * pow2(distance); }

#endif
