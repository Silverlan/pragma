#ifndef F_SH_PARALLAXMAPPING_GLS
#define F_SH_PARALLAXMAPPING_GLS

#include "/common/vertex_outputs/tangentspace.glsl"

struct ParallaxInfo {
	float heightScale;
	float steps;
};

//uniform bool u_useParallaxMap; // Defined in push constants
#ifdef GLS_FRAGMENT_SHADER
#ifndef USE_PARALLAX_MAP
#define USE_PARALLAX_MAP 1
#endif
#if USE_PARALLAX_MAP == 1
layout(LAYOUT_ID(MATERIAL, PARALLAX_MAP)) uniform sampler2D u_parallaxMap;
#endif

//Parallax Occlusion Mapping function from https://learnopengl.com/Advanced-Lighting/Parallax-Mapping
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir, ParallaxInfo parallaxInfo)
{
	const float numLayers = parallaxInfo.steps;
	const float height_scale = parallaxInfo.heightScale;
	float layerDepth = 1.0 / numLayers;
	float currentLayerDepth = 0.0;
	vec2 P = viewDir.xy * height_scale;
	vec2 deltaTexCoords = P / numLayers;
	vec2 currentTexCoords = texCoords;
	float currentDepthMapValue = 1.0 - texture(u_parallaxMap, currentTexCoords).r;

	while(currentLayerDepth < currentDepthMapValue) {
		currentTexCoords -= deltaTexCoords;
		currentDepthMapValue = 1.0 - texture(u_parallaxMap, currentTexCoords).r;
		currentLayerDepth += layerDepth;
	}

	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
	float afterDepth = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = (1.0 - texture(u_parallaxMap, prevTexCoords).r) - currentLayerDepth + layerDepth;
	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	return finalTexCoords;
}

//Corrected Viewdir calculations here
vec2 get_parallax_coordinates(bool useParallaxMap, vec2 texCoords, ParallaxInfo parallaxInfo)
{
#if USE_PARALLAX_MAP == 1
	if(useParallaxMap == false)
		return texCoords;

	vec3 T = normalize(fs_in.TBN[0]);
	vec3 B = normalize(fs_in.TBN[1]);
	vec3 N = normalize(fs_in.TBN[2]);
	mat3 tbnMatrix = transpose(mat3(T, B, N));
	vec3 camPosWs = u_renderSettings.posCam.xyz;
	vec3 vertPosWs = fs_in.vert_pos_ws;
	vec3 viewDirWs = normalize(camPosWs - vertPosWs);
	vec3 viewDirTs = normalize(tbnMatrix * viewDirWs);
	vec2 finalTexCoords = ParallaxMapping(texCoords, viewDirTs, parallaxInfo);

	return finalTexCoords;
#else
	return texCoords;
#endif
}
vec2 get_parallax_coordinates(vec2 texCoords, ParallaxInfo parallaxInfo) { return get_parallax_coordinates(true, texCoords, parallaxInfo); }

vec2 apply_parallax(bool useParallaxMap, vec2 texCoords, ParallaxInfo parallaxInfo)
{
	texCoords = get_parallax_coordinates(useParallaxMap, texCoords, parallaxInfo);
	//if(useParallaxMap == true && (texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)) // Cut off the edges
	//	discard;
	return texCoords;
}
vec2 apply_parallax(vec2 texCoords, ParallaxInfo parallaxInfo) { return apply_parallax(true, texCoords, parallaxInfo); }
#endif

#endif