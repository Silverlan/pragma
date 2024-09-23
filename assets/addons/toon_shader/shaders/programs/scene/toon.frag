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
#include "/common/inputs/fs_renderer.glsl"
#include "/common/pbr/lighting/light_spot.glsl"
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

vec3 calc_specular_component(const ShadingInfo shadingInfo, const SpecularInfo specInfo, const LightInfo lightInfo)
{
	vec3 halfVector = normalize(lightInfo.lightDirection + shadingInfo.viewDirection);
	float NdotH = max(dot(shadingInfo.surfaceNormal, halfVector), 0.0);
	float specularIntensity = pow(NdotH * lightInfo.lightIntensity, specInfo.glossiness * specInfo.glossiness);
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

void calc_toon_blinn_phong_lighting(const ShadingInfo shadingInfo, const SpecularInfo specInfo, LightSourceData lightData, const LightInfo lightInfo, const RimLightingInfo rimInfo, inout vec3 lightColor, inout vec3 specularColor, inout vec3 rimColor, uint lightIndex, bool enableShadows)
{
	float shadowFactor = 1.0;
	if(CSPEC_ENABLE_DYNAMIC_SHADOWS == 1)
		shadowFactor = enableShadows ? get_spot_light_shadow_factor(lightIndex, true) : 1.0;

	vec2 uv = vec2(1 - (lightInfo.NdotL * 0.5 + 0.5), 0.5);
	//vec4 tex = fetch_ramp_texture(vec2(0,0))
	float lightIntensity = (lightInfo.NdotL * shadowFactor) > 0 ? 1 : 0;
	//float lightIntensity = smoothstep(0, 0.01, lightInfo.NdotL * shadowFactor); // Two bands
	vec3 light = lightIntensity * lightData.color.rgb * lightInfo.lightIntensity;

	// Specular
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
	vec4 color = fetch_albedo_map(texCoords, get_instance_color());

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

			LightInfo lightInfo;
			// Default light intensities are designed for PBR, we have to
			// (subjectively) adjust using some factor.
			lightInfo.lightIntensity = light.color.a / 5.0;

			lightInfo.lightIntensity *= attenuation;
			vec3 lightDir = normalize(light.position.xyz - fs_in.vert_pos_ws.xyz);
			float NdotL = max(dot(normal, lightDir), 0.0);
			lightInfo.NdotL = NdotL;
			lightInfo.lightDirection = lightDir;

			calc_toon_blinn_phong_lighting(shadingInfo, specInfo, light, lightInfo, rimInfo, totalLightColor, totalSpecularColor, totalRimColor, lightIndex, enableShadows);
		}
	}
	if(CSPEC_ENABLE_LIGHT_SOURCES_POINT == 1) {
		for(uint i = SCENE_POINT_LIGHT_BUFFER_START; i < SCENE_POINT_LIGHT_BUFFER_END && visibleLightTileIndicesBuffer.data[tileStartOffset + i].index != -1; i++) {
			//uint lightIndex = visibleLightTileIndicesBuffer.data[tileStartOffset +i].index;
			//LightSourceData light = get_light_source(lightIndex);
			//color += apply_point_light(light,lightIndex,materialInfo,normal,view,vertPos,enableShadows);
		}
	}
	if(CSPEC_ENABLE_LIGHT_SOURCES_DIRECTIONAL == 1) {
		for(uint i = SCENE_DIRECTIONAL_LIGHT_BUFFER_START; i < SCENE_DIRECTIONAL_LIGHT_BUFFER_END && visibleLightTileIndicesBuffer.data[tileStartOffset + i].index != -1; i++) {
			//uint lightIndex = visibleLightTileIndicesBuffer.data[tileStartOffset +i].index;
			//LightSourceData light = get_light_source(lightIndex);
			//color += apply_directional_light(light,lightIndex,materialInfo,normal,view,enableShadows);
		}
	}

	// TODO
	vec3 ambientLightColor = vec3(0.4, 0.4, 0.4);

	if(CSPEC_ENABLE_IBL == 1) {
		//if(!useLightmaps)
		/*MaterialInfo matInfo;
        matInfo.perceptualRoughness = 0.5;
        matInfo.diffuseColor = vec3(1,1,1);//color.rgb;
        matInfo.specularColor = vec3(1,1,1);//specInfo.specularColor;
	    ambientLightColor = texture(u_brdfLUT, texCoords).rgb;//get_ibl_contribution(matInfo, normal, viewDir, get_reflection_probe_intensity()) *100;
    */
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
