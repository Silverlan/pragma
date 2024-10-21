#ifndef F_SH_PARALLAXMAPPING_GLS
#define F_SH_PARALLAXMAPPING_GLS

#include "/common/vertex_outputs/tangentspace.glsl"

struct ParallaxInfo {
	float heightScale;
	float steps;
};

#ifdef GLS_FRAGMENT_SHADER

//Parallax Occlusion Mapping function from https://learnopengl.com/Advanced-Lighting/Parallax-Mapping
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir, ParallaxInfo parallaxInfo)
{
#ifdef MATERIAL_PARALLAX_MAP_ENABLED
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
#else
	return texCoords;
#endif
}

//Corrected Viewdir calculations here
vec2 get_parallax_coordinates(bool useParallaxMap, vec2 texCoords, ParallaxInfo parallaxInfo)
{
#ifdef MATERIAL_PARALLAX_MAP_ENABLED
	if(useParallaxMap == false)
		return texCoords;

	mat3 tbn = get_tbn_matrix();
	vec3 T = normalize(tbn[0]);
	vec3 B = normalize(tbn[1]);
	vec3 N = normalize(tbn[2]);
	mat3 tbnMatrix = transpose(mat3(T, B, N));
	vec3 camPosWs = u_renderSettings.posCam.xyz;
	vec3 vertPosWs = get_vertex_position_ws();
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

vec2 get_uv_coordinates()
{
	vec2 texCoords = get_vertex_uv();
#ifdef MATERIAL_PARALLAX_MAP_ENABLED
	if(use_parallax_map(u_material.material.flags)) {
		ParallaxInfo parallaxInfo;
		parallaxInfo.heightScale = get_mat_parallax_height_scale();
		parallaxInfo.steps = get_mat_parallax_steps();
		texCoords = apply_parallax(true, texCoords, parallaxInfo);
	}
#endif
	return texCoords;
}

#endif

#endif