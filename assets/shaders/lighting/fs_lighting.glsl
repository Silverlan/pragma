#ifndef F_FS_LIGHTING_GLS
#define F_FS_LIGHTING_GLS

#include "../modules/sh_normalmapping.gls"
#include "../modules/fs_specularmapping.gls"
#include "/common/inputs/render_settings.glsl"

#ifndef LIGHTING_USE_INCIDENCE_ANGLE
#define LIGHTING_USE_INCIDENCE_ANGLE 1
#endif

#define DEBUG_RENDER_LIGHT_DEPTH 0 // 1 = Draw depth from shadow depth map; 2 = Draw depth from current fragment depth

#if DEBUG_RENDER_LIGHT_DEPTH != 0
#include "/functions/fs_linearize_depth.glsl"
#endif

#if ENABLE_LIGHT_DEBUG == 1
#include "../debug/fs_debug_info.gls"
#endif

#include "fs_lighting_directional.gls"
#include "fs_lighting_spot.gls"
#include "fs_lighting_point.gls"
#include "../modules/fs_ssao.gls"
#include "../modules/fs_renderer.gls"
#include "../compute/cs_forwardp_settings.gls"

vec3 test(int i, vec3 diffuseColor, vec2 texCoords, mat4 mdlMatrix, vec3 fragPosWs, vec3 fragNormal, bool bUseNormal, vec3 fragNormalWs, vec3 finalColor)
{
	LightSourceData light = get_light_source(i);
	vec3 lightPos = light.position.xyz;
	vec3 lightColor = light.color.rgb;
	vec3 fragLightColor = diffuseColor * lightColor;
	vec3 posFromWorldSpace = lightPos - fragPosWs;

	// Specular
	vec3 incidenceVector;
	if((light.flags & FLIGHT_TYPE_DIRECTIONAL) != 0)
		incidenceVector = -light.direction.xyz;
	else
		incidenceVector = normalize(posFromWorldSpace);
	vec3 specularColor = calc_phong(mdlMatrix, fragPosWs, fragNormal, incidenceVector, lightColor, texCoords);

	float cosTheta = 1.0;
	// FIXME
#if LIGHTING_USE_INCIDENCE_ANGLE == 1
	if(bUseNormal == true)
		cosTheta = calculate_normal_light_direction_angle(i, texCoords);
#else
	if(bUseNormal == true) {
		vec3 l = normalize(get_light_direction_cs(i));
		cosTheta = clamp(dot(fragNormalWs, l), 0.0, 1.0);
	}
#endif

	if((light.flags & FLIGHT_TYPE_SPOT) != 0)
		finalColor.rgb = calculate_spot_lighting(i, i, finalColor.rgb, diffuseColor, specularColor, cosTheta, posFromWorldSpace);
	else if((light.flags & FLIGHT_TYPE_POINT) != 0) {
		float lightDist = length(lightPos - fragPosWs);
		finalColor.rgb = calculate_point_lighting(i, i, finalColor.rgb, fragLightColor, specularColor, cosTheta, lightDist, fragPosWs);
	}
	else
		finalColor.rgb = calculate_directional_lighting(i, finalColor.rgb, fragLightColor, specularColor, cosTheta);
	return finalColor;
}

vec3 calculate_lighting(vec3 diffuseColor, vec2 texCoords, mat4 mdlMatrix, vec3 fragPosWs, vec3 fragNormal, bool bUseNormal, vec3 fragNormalWs)
{
	if(is_unlit() == true)
		return diffuseColor;

	vec3 ambientColor = vec3(0.0, 0.0, 0.0);
	vec3 finalColor = vec3(0.0, 0.0, 0.0);
	float ssaoFactor = 1.0;
#if ENABLE_LIGHTMAP == 1
	if(is_light_map_enabled()) {
		vec4 colLightMap = texture(u_lightMap, get_vertex_uv_lightmap());
		colLightMap.rgb /= 255.0;
		finalColor.rgb = diffuseColor * colLightMap.rgb;
	}
	else
#endif
	{
		ambientColor = diffuseColor;
		// ambientColor *= u_renderSettings.ambientColor.rgb *u_renderSettings.ambientColor.a;
		if(is_ssao_enabled() == true)
			ssaoFactor = get_ssao_occlusion(get_viewport_width(), get_viewport_height());
	}

	ivec2 location = ivec2(gl_FragCoord.xy);
	uint tileSize = get_tile_size();
	ivec2 tileID = location / ivec2(tileSize, tileSize);
	uint index = tileID.y * get_number_of_tiles_x() + tileID.x;
	uint offset = index * MAX_SCENE_LIGHTS;
	for(uint i = 0; i < MAX_SCENE_LIGHTS && visibleLightTileIndicesBuffer.data[offset + i].index != -1; i++) {
		uint lightIndex = visibleLightTileIndicesBuffer.data[offset + i].index;
		finalColor = test(int(lightIndex), diffuseColor, texCoords, mdlMatrix, fragPosWs, fragNormal, bUseNormal, fragNormalWs, finalColor);
	}
	finalColor += ambientColor;
	return finalColor * ssaoFactor;
}

vec3 calculate_lighting(vec3 diffuseColor, vec2 texCoords, mat4 mdlMatrix, vec3 fragPosWs, vec3 fragNormal, bool bUseNormal) { return calculate_lighting(diffuseColor, texCoords, mdlMatrix, fragPosWs, fragNormal, bUseNormal, vec3(0.0, 0.0, 0.0)); }
vec3 calculate_lighting(vec3 diffuseColor, vec2 texCoords, mat4 mdlMatrix, vec3 fragPosWs, vec3 fragNormal) { return calculate_lighting(diffuseColor, texCoords, mdlMatrix, fragPosWs, fragNormal, true); }
vec3 calculate_lighting(vec3 diffuseColor, vec2 texCoords, vec3 fragPosWs, vec3 fragNormal) { return calculate_lighting(diffuseColor, texCoords, mat4(1.0), fragPosWs, fragNormal); }
vec3 calculate_lighting(vec3 diffuseColor, vec2 texCoords, mat4 mdlMatrix, vec3 fragPosWs, vec3 fragNormal, vec3 fragNormalWs) { return calculate_lighting(diffuseColor, texCoords, mdlMatrix, fragPosWs, fragNormal, true, fragNormalWs); }

#endif
