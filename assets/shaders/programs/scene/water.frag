#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define PUSH_USER_CONSTANTS                                                                                                                                                                                                                                                                      \
	float waterFogIntensity;                                                                                                                                                                                                                                                                     \
	uint enableReflection;

#include "scene_push_constants.glsl"
#include "/common/fs_fog_data.glsl"
#include "/common/inputs/camera.glsl"
#include "/common/inputs/time.glsl"
#include "/common/pixel_outputs/fs_bloom_color.glsl"
#include "/common/vertex_outputs/vertex_data.glsl"

vec3 calc_light_specular(vec3 normal, vec3 viewVector, vec3 lightColor, vec3 fromLightVector)
{
	const float reflectivity = 1.0;
	const float shineDamper = 40.0;
	vec3 reflectedLight = reflect(normalize(fromLightVector), normal);
	float specular = max(dot(reflectedLight, viewVector), 0.0);
	specular = pow(specular, shineDamper);
	return lightColor * specular * reflectivity;
}

vec3 calc_light_specular(vec3 normal, vec3 viewVector)
{
	vec3 specularHighlights = vec3(0, 0, 0);

	// We don't have forward+ lighting information for water (due to the translucency and, as a result, it not being included in the prepass depth buffer),
	// so highlights are disabled for now.
	/*if(CSPEC_ENABLE_LIGHT_SOURCES == 1 && CSPEC_ENABLE_DYNAMIC_LIGHTING == 1)
	{
        ivec2 location = ivec2(gl_FragCoord.xy);
        uint tileSize = get_tile_size();
        ivec2 tileID = location /ivec2(tileSize,tileSize);
        uint index = tileID.y *get_number_of_tiles_x() +tileID.x;
        uint tileStartOffset = index *MAX_SCENE_LIGHTS;
		if(CSPEC_ENABLE_LIGHT_SOURCES_SPOT == 1)
		{
			for(uint i=SCENE_SPOT_LIGHT_BUFFER_START; i < SCENE_SPOT_LIGHT_BUFFER_END && visibleLightTileIndicesBuffer.data[tileStartOffset + i].index != -1; i++)
			{
				uint lightIndex = visibleLightTileIndicesBuffer.data[tileStartOffset +i].index;
				LightSourceData light = get_light_source(lightIndex);
				//if(useLightmaps == true && (light.flags &FLIGHT_BAKED_LIGHT_SOURCE) != 0)
				//    continue;
				specularHighlights += calc_light_specular(normal,viewVector,light.color.rgb,fs_in.vert_pos_ws -light.position.xyz);
			}
		}
		if(CSPEC_ENABLE_LIGHT_SOURCES_POINT == 1)
		{
			for(uint i=SCENE_POINT_LIGHT_BUFFER_START; i < SCENE_POINT_LIGHT_BUFFER_END && visibleLightTileIndicesBuffer.data[tileStartOffset + i].index != -1; i++)
			{
				uint lightIndex = visibleLightTileIndicesBuffer.data[tileStartOffset +i].index;
				LightSourceData light = get_light_source(lightIndex);
				//if(useLightmaps == true && (light.flags &FLIGHT_BAKED_LIGHT_SOURCE) != 0)
				//    continue;
				specularHighlights += calc_light_specular(normal,viewVector,light.color.rgb,fs_in.vert_pos_ws -light.position.xyz);
			}
		}
		if(CSPEC_ENABLE_LIGHT_SOURCES_DIRECTIONAL == 1)
		{
			for(uint i=SCENE_DIRECTIONAL_LIGHT_BUFFER_START; i < SCENE_DIRECTIONAL_LIGHT_BUFFER_END && visibleLightTileIndicesBuffer.data[tileStartOffset + i].index != -1; i++)
			{
				uint lightIndex = visibleLightTileIndicesBuffer.data[tileStartOffset +i].index;
				LightSourceData light = get_light_source(lightIndex);
				//if(useLightmaps == true && (light.flags &FLIGHT_BAKED_LIGHT_SOURCE) != 0)
				//    continue;
				specularHighlights += calc_light_specular(normal,viewVector,light.color.rgb,light.direction.xyz);
			}
		}
	}*/
	return specularHighlights;
}

layout(LAYOUT_ID(WATER, REFLECTION_MAP)) uniform sampler2D u_reflectionMap;
layout(LAYOUT_ID(WATER, REFRACTION_MAP)) uniform sampler2D u_refractionMap;
layout(LAYOUT_ID(WATER, REFRACTION_DEPTH)) uniform sampler2D u_refractionDepthMap;

layout(std140, LAYOUT_ID(WATER, SETTINGS)) uniform WaterSettings
{
	float waterScale; // TODO: Remove me, let mapper decide!
	float waveStrength;
	float waveSpeed; // Wave direction?
	float reflectiveIntensity;
}
u_water;

layout(std140, LAYOUT_ID(WATER, FOG)) uniform UWaterFog { Fog fog; }
u_waterFog;

#define DEBUG_WATER_MODE_NONE 0
#define DEBUG_WATER_MODE_REFRACTION 1
#define DEBUG_WATER_MODE_REFLECTION 2

#define DEBUG_WATER_MODE DEBUG_WATER_MODE_NONE

void main()
{
	vec3 vertPos = get_vertex_position_ws();
	vec4 coords = u_camera.VP * vec4(vertPos, 1.0);
	coords.xy /= coords.w;
	coords.x = coords.x * 0.5 + 0.5;
	coords.y = coords.y * 0.5 + 0.5;

	vec2 reflectUv = coords.xy;
	reflectUv = clamp(reflectUv, 0.001, 0.999);
#if DEBUG_WATER_MODE == DEBUG_WATER_MODE_REFRACTION
	fs_color = texture(u_refractionMap, reflectUv);
#elif DEBUG_WATER_MODE == DEBUG_WATER_MODE_REFLECTION
	fs_color = texture(u_reflectionMap, reflectUv);
	fs_color.a = 1;
#else
	vec4 wposWorld = inverse(get_view_matrix()) * get_model_matrix() * vec4(vertPos, 1.0);

	float waterScale = u_water.waterScale;
	float waveStrength = u_water.waveStrength;
	float waveSpeed = 0.03 * u_water.waveSpeed * u_water.waterScale;
	float reflectiveIntensity = u_water.reflectiveIntensity;

	vec2 refractUv = reflectUv;

	float depthWaterFloor = get_depth_distance(texture(u_refractionDepthMap, refractUv).x, u_renderSettings.nearZ, u_renderSettings.farZ);
	float depthWaterSurf = gl_FragCoord.z / gl_FragCoord.w;
	float depthWater = depthWaterFloor - depthWaterSurf;

	float t = mod(cur_time() * waveSpeed, 1.0) / 1.0;
	vec2 uv = get_vertex_uv() * waterScale;
	vec2 dudvCoord = (texture(u_dudvMap, vec2(uv.x + t, uv.y)).rg * 0.1);
	dudvCoord = uv + vec2(dudvCoord.x, dudvCoord.y + t);
	vec2 distortion = (texture(u_dudvMap, dudvCoord).rg * 2.0 - 1.0) * waveStrength;

	refractUv += distortion;
	refractUv = clamp(refractUv, 0.001, 0.999);

	reflectUv += distortion;
	reflectUv = clamp(reflectUv, 0.001, 0.999);

	vec4 refractColor = texture(u_refractionMap, refractUv);

	// Apply water fog
	Fog fog = u_waterFog.fog;
	refractColor.rgb = mix(refractColor.rgb, u_waterFog.fog.color.rgb, get_fog_factor(fog, depthWater) * u_pushConstants.waterFogIntensity);

	vec3 viewVector = normalize(u_renderSettings.posCam.xyz - vertPos.xyz);
	vec3 vertNormal = (get_model_matrix() * vec4(get_vertex_normal(), 0.0)).xyz;
	float refractiveFactor = dot(viewVector, vertNormal);
	refractiveFactor = pow(refractiveFactor, reflectiveIntensity);

	// Lighting
	vec4 nmCol = texture(u_normalMap, dudvCoord);
	vec3 normal = vec3(nmCol.r * 2.0 - 1.0, nmCol.b, nmCol.g * 2.0 - 1.0);
	normal = normalize(normal);

	vec3 specularHighlights = calc_light_specular(normal, viewVector);

	if(u_pushConstants.enableReflection == 1) {
		vec4 reflectColor = texture(u_reflectionMap, reflectUv);
		reflectColor.rgb = mix(reflectColor.rgb, u_waterFog.fog.color.rgb, 0.6);
		fs_color = mix(reflectColor, refractColor, refractiveFactor);
	}
	else
		fs_color = texture(u_reflectionMap, reflectUv);

	fs_color.a = clamp(depthWater / 20.0, 0.0, 1.0);
#endif
}
