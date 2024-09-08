#ifndef F_FS_PBR_LIGHTING_DIRECT_GLS
#define F_FS_PBR_LIGHTING_DIRECT_GLS

#include "/common/pbr/material.glsl"
#include "/common/inputs/fs_renderer.glsl"
#include "light_spot_shadow.glsl"
#include "light_point.glsl"
#include "light_directional.glsl"

#ifndef PR_F_CALC_PBR_DIRECT_LIGHTING_DEFINED
vec3 calc_pbr_direct_lighting(MaterialInfo materialInfo, vec3 normal, vec3 view, vec3 vertPos, bool enableShadows)
{
	vec3 color = vec3(0.0, 0.0, 0.0);
	ivec2 location = ivec2(gl_FragCoord.xy);
	uint tileSize = get_tile_size();
	ivec2 tileID = location / ivec2(tileSize, tileSize);
	uint index = tileID.y * get_number_of_tiles_x() + tileID.x;
	uint tileStartOffset = index * MAX_SCENE_LIGHTS;
	if(CSPEC_ENABLE_LIGHT_SOURCES_SPOT == 1) {
		for(uint i = SCENE_SPOT_LIGHT_BUFFER_START; i < SCENE_SPOT_LIGHT_BUFFER_END && visibleLightTileIndicesBuffer.data[tileStartOffset + i].index != -1; i++) {
			uint lightIndex = visibleLightTileIndicesBuffer.data[tileStartOffset + i].index;
			LightSourceData light = get_light_source(lightIndex);
			//if(useLightmaps == true && (light.flags &FLIGHT_BAKED_LIGHT_SOURCE) != 0)
			//	continue;
			color += apply_spot_light(light, lightIndex, materialInfo, normal, view, vertPos, enableShadows);
		}
	}
	if(CSPEC_ENABLE_LIGHT_SOURCES_POINT == 1) {
		for(uint i = SCENE_POINT_LIGHT_BUFFER_START; i < SCENE_POINT_LIGHT_BUFFER_END && visibleLightTileIndicesBuffer.data[tileStartOffset + i].index != -1; i++) {
			uint lightIndex = visibleLightTileIndicesBuffer.data[tileStartOffset + i].index;
			LightSourceData light = get_light_source(lightIndex);
			//if(useLightmaps == true && (light.flags &FLIGHT_BAKED_LIGHT_SOURCE) != 0)
			//	continue;
			color += apply_point_light(light, lightIndex, materialInfo, normal, view, vertPos, enableShadows);
		}
	}
	if(CSPEC_ENABLE_LIGHT_SOURCES_DIRECTIONAL == 1) {
		for(uint i = SCENE_DIRECTIONAL_LIGHT_BUFFER_START; i < SCENE_DIRECTIONAL_LIGHT_BUFFER_END && visibleLightTileIndicesBuffer.data[tileStartOffset + i].index != -1; i++) {
			uint lightIndex = visibleLightTileIndicesBuffer.data[tileStartOffset + i].index;
			LightSourceData light = get_light_source(lightIndex);
			//if(useLightmaps == true && (light.flags &FLIGHT_BAKED_LIGHT_SOURCE) != 0)
			//	continue;
			color += apply_directional_light(light, lightIndex, materialInfo, normal, view, enableShadows);
		}
	}
	return color;
}
#endif

#endif
