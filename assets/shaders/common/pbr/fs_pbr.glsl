#ifndef F_FS_PBR_GLS
#define F_FS_PBR_GLS

#define ENABLE_LIGHT_DEBUG 1
#ifndef DEBUG_MODE
#define DEBUG_MODE DEBUG_MODE_NONE
#endif

#define DEBUG_FORWARDP_OUTPUT_MODE_DEPTH_NONE 0
#define DEBUG_FORWARDP_OUTPUT_MODE_DEPTH_BUFFER 1
#define DEBUG_FORWARDP_OUTPUT_MODE_UV_X 2
#define DEBUG_FORWARDP_OUTPUT_MODE_UV_Y 3
#define DEBUG_FORWARDP_OUTPUT_MODE_LOCATION_X 4
#define DEBUG_FORWARDP_OUTPUT_MODE_LOCATION_Y 5

#define DEBUG_FORWARDP_OUTPUT_MODE DEBUG_FORWARDP_OUTPUT_MODE_DEPTH_NONE

#include "material.glsl"
#include "/common/fs_tonemapping.glsl"
#include "/common/inputs/textures/parallax_map.glsl"
#include "/common/inputs/textures/emission_map.glsl"
#include "/common/inputs/fs_renderer.glsl"
#include "/programs/scene/pbr/fs_util.glsl"
#include "/functions/fs_linearize_depth.glsl"
#include "/util/fs_ssao.glsl"
#include "fs_lighting.glsl"
#include "fs_debug_csm.glsl"
#include "fs_debug.glsl"
#include "fs_util.glsl"

void apply_debug_heatmap_color(uint istart, uint iend, inout vec3 color)
{
	ivec2 location = ivec2(gl_FragCoord.xy);
	uint tileSize = get_tile_size();
	ivec2 tileID = location / ivec2(tileSize, tileSize);
	uint index = tileID.y * get_number_of_tiles_x() + tileID.x;
	uint tileStartOffset = index * MAX_SCENE_LIGHTS;
	uint numLights = 0;
	for(uint i = istart; i < iend && visibleLightTileIndicesBuffer.data[tileStartOffset + i].index != -1; i++) {
		uint lightIndex = visibleLightTileIndicesBuffer.data[tileStartOffset + i].index;
		LightSourceData light = get_light_source(int(lightIndex));
		if((light.flags & FLIGHT_TYPE_DIRECTIONAL) == 0)
			++numLights;
	}
	if(numLights > 0) {
		const float dbgIntensity = 0.95;
		color.rgb = (vec3(tileID.x / float(tileSize), tileID.y / float(tileSize), 0) * (numLights / float(5))) * pow(1.0, dbgIntensity * 1.0) + color.rgb * (1.0 - dbgIntensity);
	}
}

vec4 calc_pbr(vec4 albedoColor, vec2 texCoords, uint debugMode, PbrMaterial pbrMat, vec3 materialEmissionFactor, uint materialFlags)
{
	// Metallic and Roughness material properties are packed together
	// In glTF, these factors can be specified by fixed scalar values
	// or from a metallic-roughness map
	float perceptualRoughness = 0.0;
	float metallic = 0.0;
	vec4 baseColor = vec4(0.0, 0.0, 0.0, 1.0);
	vec3 diffuseColor = vec3(0.0);
	vec3 specularColor = vec3(0.0);
	vec3 f0 = vec3(0.04);

	// f0 = specular
	//specularColor = f0;
	//float oneMinusSpecularStrength = 1.0 - max(max(f0.r, f0.g), f0.b);
	//diffuseColor = baseColor.rgb * oneMinusSpecularStrength;

	vec4 rma;
#ifdef MATERIAL_RMA_MAP_ENABLED
	if(use_rma_map(materialFlags))
		rma = fetch_rma_map(texCoords);
	else
#endif
		rma = vec4(1 /* ao */, 1 /* roughness */, 1 /*metalness */, 1);
	perceptualRoughness = rma[RMA_CHANNEL_ROUGHNESS] * 1.0;

	// Glossiness
	// TODO
	//vec4 glossiness = texture(u_specularMap,texCoords);
	//perceptualRoughness *= (1.0 -glossiness.r); // Glossiness to roughness

	perceptualRoughness *= pbrMat.roughnessFactor;

	metallic = rma[RMA_CHANNEL_METALNESS] * pbrMat.metalnessFactor;

	baseColor = albedoColor;
#if defined(MATERIAL_WRINKLE_COMPRESS_MAP_ENABLED) && defined(MATERIAL_WRINKLE_STRETCH_MAP_ENABLED)
	if(use_wrinkle_maps(materialFlags)) {
		if(fs_in.wrinkleDelta != 0.0) {
			float wrinkle = clamp(-fs_in.wrinkleDelta, 0, 1);
			float stretch = clamp(fs_in.wrinkleDelta, 0, 1);
			float baseColorFactor = 1.0 - wrinkle - stretch;

			vec4 wrinkleCol = fetch_wrinkle_compress_map(texCoords);
			vec4 stretchCol = fetch_wrinkle_stretch_map(texCoords);

			baseColor.rgb = baseColorFactor * baseColor.rgb + wrinkle * wrinkleCol.rgb + stretch * stretchCol.rgb;
		}
	}
#endif

	//baseColor *= u_DiffuseFactor;
	diffuseColor = baseColor.rgb * (vec3(1.0) - f0) * (1.0 - metallic);
	specularColor = mix(f0, baseColor.rgb, metallic);

	perceptualRoughness = clamp(perceptualRoughness, 0.0, 1.0);
	metallic = clamp(metallic, 0.0, 1.0);

	// Roughness is authored as perceptual roughness; as is convention,
	// convert to material roughness by squaring the perceptual roughness [2].
	float alphaRoughness = perceptualRoughness * perceptualRoughness;

	// Compute reflectance.
	float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);

	vec3 specularEnvironmentR0 = specularColor.rgb;
	// Anything less than 2% is physically impossible and is instead considered to be shadowing. Compare to "Real-Time-Rendering" 4th editon on page 325.
	vec3 specularEnvironmentR90 = vec3(clamp(reflectance * 50.0, 0.0, 1.0));

	float aoFactor = pbrMat.aoFactor;
	MaterialInfo materialInfo = MaterialInfo(perceptualRoughness, specularEnvironmentR0, alphaRoughness, diffuseColor, specularEnvironmentR90, specularColor, aoFactor);

	// Lighting
	vec3 color;
#if DEBUG_FORWARDP_OUTPUT_MODE != DEBUG_FORWARDP_OUTPUT_MODE_NONE
	ivec2 location = ivec2(gl_FragCoord.xy);
	uint tileSize = get_tile_size();
	ivec2 tileID = location / ivec2(tileSize, tileSize);
	uint index = tileID.y * get_number_of_tiles_x() + tileID.x;
	uint tileStartOffset = index * MAX_SCENE_LIGHTS;
#if DEBUG_FORWARDP_OUTPUT_MODE == DEBUG_FORWARDP_OUTPUT_MODE_DEPTH_BUFFER
	float d = intBitsToFloat(visibleLightTileIndicesBuffer.data[tileStartOffset].index);
	d = get_linearized_depth(d, u_renderSettings.nearZ, u_renderSettings.farZ);
	color = vec3(d, d, d);
#elif DEBUG_FORWARDP_OUTPUT_MODE == DEBUG_FORWARDP_OUTPUT_MODE_UV_X || DEBUG_FORWARDP_OUTPUT_MODE == DEBUG_FORWARDP_OUTPUT_MODE_UV_Y
	float uvx = intBitsToFloat(visibleLightTileIndicesBuffer.data[tileStartOffset].index);
	color = vec3(uvx, uvx, uvx);
#elif DEBUG_FORWARDP_OUTPUT_MODE == DEBUG_FORWARDP_OUTPUT_MODE_LOCATION_X || DEBUG_FORWARDP_OUTPUT_MODE == DEBUG_FORWARDP_OUTPUT_MODE_LOCATION_Y
	int v = visibleLightTileIndicesBuffer.data[tileStartOffset].index;
	color = vec3(v, v, v) / 5000.0;
#endif
#else
	if(CSPEC_DEBUG_MODE_ENABLED == 0)
		color = calc_pbr_lighting(texCoords, materialInfo, materialFlags, baseColor);
	else {
		if((debugMode & DEBUG_MODE_UNLIT) != 0)
			color = baseColor.rgb;
		else
			color = calc_pbr_lighting(texCoords, materialInfo, materialFlags, baseColor);
		if((debugMode & DEBUG_MODE_FORWARD_PLUS_HEATMAP) != 0) {
			apply_debug_heatmap_color(SCENE_SPOT_LIGHT_BUFFER_START, SCENE_SPOT_LIGHT_BUFFER_END, color);
			apply_debug_heatmap_color(SCENE_POINT_LIGHT_BUFFER_START, SCENE_POINT_LIGHT_BUFFER_END, color);
			apply_debug_heatmap_color(SCENE_DIRECTIONAL_LIGHT_BUFFER_START, SCENE_DIRECTIONAL_LIGHT_BUFFER_END, color);
		}
	}
#endif

	// Ao
	float ao = rma[RMA_CHANNEL_AO];

	// TODO
	if(CSPEC_ENABLE_SSAO == 1) {
		if(is_ssao_enabled() == true)
			ao *= get_ssao_occlusion(get_viewport_width(), get_viewport_height()); // TODO: Disable ssao if ao map exists?
	}

	// Apply optional PBR terms for additional (optional) shading
	color = mix(color, color * ao, materialInfo.aoFactor * 0.7 + 0.3);

	// regular shading
	vec4 result = vec4(color.rgb, baseColor.a);

	if(use_glow_map(materialFlags))
		result = get_emission_color(result, baseColor, texCoords, materialEmissionFactor, materialFlags);

	if(CSPEC_DEBUG_MODE_ENABLED == 1)
		result = apply_debug_mode(materialFlags, debugMode, result, texCoords, diffuseColor, specularColor, metallic, perceptualRoughness, aoFactor, reflectance);
	return result;
}

#endif
