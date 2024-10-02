/*
    Copyright (C) 2024 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "/common/light_source.glsl"
#include "/common/vertex_outputs/vertex_data.glsl"
#include "/common/inputs/textures/normal_map.glsl"
#include "/common/inputs/fs_lightmap.glsl"
#include "/common/inputs/fs_renderer.glsl"
#include "/common/pbr/lighting/light_spot.glsl"
#include "/common/pbr/lighting/light_point.glsl"
#include "/common/pbr/lighting/light_directional.glsl"
#include "/common/pixel_outputs/fs_bloom_color.glsl"
#include "/programs/scene/pbr/fs_config.glsl"
#include "/programs/scene/scene_push_constants.glsl"
#include "/lighting/fs_lighting_spot.glsl"

// See https://roystan.net/articles/toon-shader/

struct RimLightingInfo {
	float amount;
	float threshold;
	vec3 color;
};

struct SpecularInfo {
	float glossiness;
	vec3 specularColor;
};

struct ShadingInfo {
	vec3 surfaceNormal;
	vec3 viewDirection;
};

struct LightInfo {
	vec3 lightDirection;
	float lightIntensity;
	float NdotL; // The dot product of the normal and the light direction
};

layout(LAYOUT_ID(RAMP, TEXTURE)) uniform sampler2D u_rampTexture;
vec3 calc_specular_component(const ShadingInfo shadingInfo, const SpecularInfo specInfo, const LightInfo lightInfo)
{
	vec3 halfVector = normalize(lightInfo.lightDirection + shadingInfo.viewDirection);
	float NdotH = max(dot(shadingInfo.surfaceNormal, halfVector), 0.0);
	float specularIntensity = pow(NdotH * lightInfo.lightIntensity, specInfo.glossiness *specInfo.glossiness);
	float specularIntensitySmooth = smoothstep(0.005, 0.01, specularIntensity);
	return specularIntensitySmooth * specInfo.specularColor;
}


vec3 calc_rim_lighting(const ShadingInfo shadingInfo, const LightInfo lightInfo, const RimLightingInfo rimInfo)
{
	float rimDot = 1 - dot(shadingInfo.viewDirection, shadingInfo.surfaceNormal);
	float rimIntensity = rimDot * pow(lightInfo.NdotL, rimInfo.threshold);
	rimIntensity = smoothstep(rimInfo.amount - 0.01, rimInfo.amount + 0.01, rimIntensity);
	return rimIntensity * rimInfo.color;
}

vec3 calc_light_color(float NdotL, vec3 lightColor, float lightIntensity, float shadowFactor)
{
	vec2 uv = vec2(1 - (NdotL * 0.5 + 0.5), 0.5);
	vec4 rampCol = texture(u_rampTexture, uv);
	return lightColor * lightIntensity *rampCol.rgb *shadowFactor;
}

void calc_toon_blinn_phong_lighting(const ShadingInfo shadingInfo, const SpecularInfo specInfo, LightSourceData lightData, const LightInfo lightInfo, float shadowFactor, const RimLightingInfo rimInfo, inout vec3 lightColor, inout vec3 specularColor, inout vec3 rimColor, uint lightIndex, bool enableShadows)
{
	vec3 light = calc_light_color(lightInfo.NdotL, lightData.color.rgb, lightInfo.lightIntensity, shadowFactor);
	vec3 specular = calc_specular_component(shadingInfo, specInfo, lightInfo);
	vec3 rim = calc_rim_lighting(shadingInfo, lightInfo, rimInfo);

	lightColor += light.rgb;
	specularColor += specular;
	rimColor += rim;
}

#define TOON_DEBUG_MODE_NONE 0
#define TOON_DEBUG_MODE_ALBEDO 1
#define TOON_DEBUG_MODE_AMBIENT_LIGHT 2
#define TOON_DEBUG_MODE_DIRECT_LIGHT 3
#define TOON_DEBUG_MODE_SPECULAR 4
#define TOON_DEBUG_MODE_RIM 5

const uint TOON_DEBUG_MODE = TOON_DEBUG_MODE_NONE;

LightInfo calc_light_info(float lightIntensity, vec3 normal, vec3 lightDir)
{
	LightInfo lightInfo;
	// Default light intensities are designed for PBR, we have to
	// (subjectively) adjust using some factor.
	lightInfo.lightIntensity = lightIntensity;
	
	float NdotL = max(dot(normal, lightDir), 0.0);
	lightInfo.NdotL = NdotL;
	lightInfo.lightDirection = lightDir;
	return lightInfo;
}

void calc_toon_blinn_phong_lighting(ShadingInfo shadingInfo, SpecularInfo specInfo, RimLightingInfo rimInfo, LightSourceData light, vec3 lightDir, float attenuation, float shadowFactor, vec3 normal, inout vec3 lightColor, inout vec3 specularColor, inout vec3 rimColor, uint lightIndex, bool enableShadows)
{
	LightInfo lightInfo = calc_light_info((light.color.a / 5.0) *attenuation, normal, lightDir);
	calc_toon_blinn_phong_lighting(shadingInfo, specInfo, light, lightInfo, shadowFactor, rimInfo, lightColor, specularColor, rimColor, lightIndex, enableShadows);
}

void main()
{
	SpecularInfo specInfo;
	specInfo.specularColor = get_mat_specular_color();
	specInfo.glossiness = get_mat_glossiness();

	RimLightingInfo rimInfo;
	rimInfo.amount = get_mat_rim_amount();
	rimInfo.threshold = 0.1;
	rimInfo.color = get_mat_rim_color();

	vec2 texCoords = fs_in.vert_uv;
	vec4 baseColor = fetch_albedo_map(texCoords, get_instance_color());
	vec4 color = baseColor;

	mat3 normalMatrix = transpose(inverse(mat3(fs_in.M)));
	vec3 normal = get_normal_from_map(texCoords, get_mat_flags());
	normal = normalize(normalMatrix * normal);

	vec3 viewDir = normalize(u_renderSettings.posCam.xyz - fs_in.vert_pos_ws.xyz);
	ShadingInfo shadingInfo;
	shadingInfo.surfaceNormal = normal;
	shadingInfo.viewDirection = viewDir;

	vec3 totalLightColor = vec3(0, 0, 0);
	vec3 totalSpecularColor = vec3(0, 0, 0);
	vec3 totalRimColor = vec3(0, 0, 0);

	ivec2 location = ivec2(gl_FragCoord.xy);
	uint tileSize = get_tile_size();
	ivec2 tileID = location / ivec2(tileSize, tileSize);
	uint index = tileID.y * get_number_of_tiles_x() + tileID.x;
	uint tileStartOffset = index * MAX_SCENE_LIGHTS;
	bool enableShadows = are_shadows_enabled();
	if(CSPEC_ENABLE_LIGHT_SOURCES_SPOT == 1) {
		for(uint i = SCENE_SPOT_LIGHT_BUFFER_START; i < SCENE_SPOT_LIGHT_BUFFER_END && visibleLightTileIndicesBuffer.data[tileStartOffset + i].index != -1; i++) {
			uint lightIndex = visibleLightTileIndicesBuffer.data[tileStartOffset + i].index;
			LightSourceData light = get_light_source(lightIndex);
			float attenuation = calc_spot_light_attenuation(light, fs_in.vert_pos_ws.xyz);
			vec3 lightDir = normalize(light.position.xyz - fs_in.vert_pos_ws.xyz);
			float shadowFactor = 1.0;
			if(CSPEC_ENABLE_DYNAMIC_SHADOWS == 1)
				shadowFactor = enableShadows ? get_spot_light_shadow_factor(lightIndex, true) : 1.0;
			calc_toon_blinn_phong_lighting(shadingInfo, specInfo, rimInfo, light, lightDir, attenuation, shadowFactor, normal, totalLightColor, totalSpecularColor, totalRimColor, lightIndex, enableShadows);
		}
	}
	if(CSPEC_ENABLE_LIGHT_SOURCES_POINT == 1) {
		for(uint i = SCENE_POINT_LIGHT_BUFFER_START; i < SCENE_POINT_LIGHT_BUFFER_END && visibleLightTileIndicesBuffer.data[tileStartOffset + i].index != -1; i++) {
			uint lightIndex = visibleLightTileIndicesBuffer.data[tileStartOffset + i].index;
			LightSourceData light = get_light_source(lightIndex);
			float attenuation = calc_point_light_attenuation(light, fs_in.vert_pos_ws.xyz);
			vec3 lightDir = normalize(light.position.xyz - fs_in.vert_pos_ws.xyz);
			float shadowFactor = 1.0;
			if(CSPEC_ENABLE_DYNAMIC_SHADOWS == 1)
				shadowFactor = enableShadows ? get_point_light_shadow_factor(lightIndex, true, fs_in.vert_pos_ws.xyz) : 1.0;
			calc_toon_blinn_phong_lighting(shadingInfo, specInfo, rimInfo, light, lightDir, attenuation, shadowFactor, normal, totalLightColor, totalSpecularColor, totalRimColor, lightIndex, enableShadows);
		}
	}
	if(CSPEC_ENABLE_LIGHT_SOURCES_DIRECTIONAL == 1) {
		for(uint i = SCENE_DIRECTIONAL_LIGHT_BUFFER_START; i < SCENE_DIRECTIONAL_LIGHT_BUFFER_END && visibleLightTileIndicesBuffer.data[tileStartOffset + i].index != -1; i++) {
			uint lightIndex = visibleLightTileIndicesBuffer.data[tileStartOffset + i].index;
			LightSourceData light = get_light_source(lightIndex);
			float attenuation = 1.0;
			vec3 lightDir = -light.direction.xyz;
			float shadowFactor = 1.0;
			if(CSPEC_ENABLE_DYNAMIC_SHADOWS == 1)
				shadowFactor = enableShadows ? get_directional_light_shadow_factor(lightIndex) : 1.0;
			calc_toon_blinn_phong_lighting(shadingInfo, specInfo, rimInfo, light, lightDir, attenuation, shadowFactor, normal, totalLightColor, totalSpecularColor, totalRimColor, lightIndex, enableShadows);
		}
	}

	// TODO
	vec3 ambientLightColor = vec3(0.4, 0.4, 0.4);

	bool useLightmaps = false;
	if(CSPEC_ENABLE_LIGHT_MAPS == 1) {
		useLightmaps = is_light_map_enabled();
		//if(useLightmaps)
		//	color = baseColor.rgb;
	}
	if(CSPEC_ENABLE_IBL == 1) {
		//if(!useLightmaps)
		/*MaterialInfo matInfo;
        matInfo.perceptualRoughness = 0.5;
        matInfo.diffuseColor = vec3(1,1,1);//color.rgb;
        matInfo.specularColor = vec3(1,1,1);//specInfo.specularColor;
	    ambientLightColor = texture(u_brdfLUT, texCoords).rgb;//get_ibl_contribution(matInfo, normal, viewDir, get_reflection_probe_intensity()) *100;
    */
	}
	if(CSPEC_ENABLE_LIGHT_MAPS == 1) {
		// TODO: Lightmap mode should be determined by specialization constant to avoid if-condition overhead
		if(useLightmaps) {
			vec4 colDirect = texture(u_lightMap, fs_in.vert_uv_lightmap.xy);
			float exposure = get_lightmap_exposure_pow();
			if(is_indirect_light_map_enabled()) {
				vec3 colIndirect = texture(u_lightMapIndirect, fs_in.vert_uv_lightmap.xy).rgb;
				if(is_directional_light_map_enabled()) {
					vec3 dominantDir = texture(u_lightMapDominant, fs_in.vert_uv_lightmap.xy).rgb;
					dominantDir = dominantDir * 2.0 - 1.0;
					dominantDir = normalize(dominantDir);

					LightInfo lightInfo = calc_light_info(1.0, normal, dominantDir);
					vec3 light = calc_light_color(lightInfo.NdotL, colDirect.rgb, lightInfo.lightIntensity, 1.0);
					vec3 specular = calc_specular_component(shadingInfo, specInfo, lightInfo);
					vec3 rim = calc_rim_lighting(shadingInfo, lightInfo, rimInfo);

					totalLightColor += light.rgb;
					totalSpecularColor += specular;
					totalRimColor += rim;
				}
				else
					color.rgb = baseColor.rgb * ((colDirect.rgb + colIndirect.rgb) * exposure);
			}
			else
				color.rgb += baseColor.rgb * (exposure * colDirect.rgb);
		}
	}

	if(TOON_DEBUG_MODE == TOON_DEBUG_MODE_NONE)
		color.rgb *= (ambientLightColor + totalLightColor + totalSpecularColor + totalRimColor);
	else if(TOON_DEBUG_MODE == TOON_DEBUG_MODE_ALBEDO)
		;
	else if(TOON_DEBUG_MODE == TOON_DEBUG_MODE_AMBIENT_LIGHT)
		color = vec4(ambientLightColor, 1);
	else if(TOON_DEBUG_MODE == TOON_DEBUG_MODE_DIRECT_LIGHT)
		color = vec4(totalLightColor, 1);
	else if(TOON_DEBUG_MODE == TOON_DEBUG_MODE_SPECULAR)
		color = vec4(totalSpecularColor, 1);
	else if(TOON_DEBUG_MODE == TOON_DEBUG_MODE_RIM)
		color = vec4(totalRimColor, 1);

	fs_color = color;
	extract_bright_color(color);
}
