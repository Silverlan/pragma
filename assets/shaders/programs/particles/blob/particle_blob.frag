#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

bool is_material_translucent(uint flags) {return false;}

#include "math_scalar_field.glsl"
#include "../particle_instance.glsl"
#include "../particle_alpha_mode.glsl"
#include "../particle_render_flags.glsl"
#include "/lighting/inputs/light_sources.glsl"
#include "/common/color.glsl"
#include "/common/depth.glsl"
#include "/common/inputs/camera.glsl"
#include "/common/inputs/render_settings.glsl"
#include "/common/inputs/fs_renderer.glsl"
#include "/common/limits.glsl"
#include "/common/light_source.glsl"
#include "/common/pbr/material.glsl"
#include "/common/pbr/lighting/ibl.glsl"
#include "/common/pbr/lighting/lighting_direct.glsl"

layout(location = 0) out vec4 fs_color;
layout(location = 1) out vec4 fs_brightColor;

vec4 calc_interpolated_color(vec3 fragPosWs)
{
	float totalDist = 0.0;
	float distances[8];

	// Calculate distance sum
	for(int i = 0; i < 4; ++i) {
		uint v = get_adjacent_blob_index(i);

		uint id0 = v & TWO_BYTE_MASK;
		if(id0 == TWO_BYTE_MASK)
			break; // There are no remaining neighbors
		totalDist += distances[i * 2] = si_linear(fragPosWs, u_particleData.particles[id0].position.xyz, u_particleData.particles[id0].position.w);

		uint id1 = (v >> 16) & TWO_BYTE_MASK;
		if(id1 == TWO_BYTE_MASK)
			break; // There are no remaining neighbors
		totalDist += distances[i * 2 + 1] = si_linear(fragPosWs, u_particleData.particles[id1].position.xyz, u_particleData.particles[id1].position.w);
	}
	//

	vec4 colResult = vec4(0.0, 0.0, 0.0, 1.0);
	for(int i = 0; i < 4; ++i) {
		uint v = get_adjacent_blob_index(i);

		uint id0 = v & TWO_BYTE_MASK;
		if(id0 == TWO_BYTE_MASK)
			break; // There are no remaining neighbors
		vec4 col0 = get_particle_blob_color(id0);
		float sc0 = distances[i * 2] / totalDist;
		colResult.rgb += rgb_to_hsv(col0.rgb) * sc0;
		colResult.a += col0.a * sc0;

		uint id1 = (v >> 16) & TWO_BYTE_MASK;
		if(id1 == TWO_BYTE_MASK)
			break; // There are no remaining neighbors
		vec4 col1 = get_particle_blob_color(id1);
		float sc1 = distances[i * 2 + 1] / totalDist;
		colResult.rgb += rgb_to_hsv(col1.rgb) * sc1;
		colResult.a += col1.a * sc1;
	}
	return vec4(hsv_to_rgb(colResult.rgb), colResult.a);
}

const uint DEBUG_NONE = 0;
const uint DEBUG_EYE_DIR = 1;
const uint DEBUG_SURFACE_NORMAL = 2;
const uint DEBUG_FLAT_COLOR = 3;

void main()
{
	vec3 eyeDir = normalize(fs_in.vert_pos_cs.xyz);
	uint debugMode = DEBUG_NONE; // u_instance.debugMode
	if(debugMode == DEBUG_EYE_DIR) {
		fs_color = vec4(abs(eyeDir.x), abs(eyeDir.y), abs(eyeDir.z), 1.0);
		fs_brightColor = fs_color;
		gl_FragDepth = gl_FragCoord.z;
		return;
	}

	vec3 intersection;
	mat4 vm = get_view_matrix();
	float scalarFieldValue = calc_scalar_field_value(vm, fs_in.vert_pos_cs.xyz, eyeDir, intersection);
	if(scalarFieldValue == 0.0)
		discard;

	if(debugMode == DEBUG_SURFACE_NORMAL) {
		vec3 fragNormal = calc_implicit_area_normal(vm, intersection);
		fs_color = vec4(fragNormal, 1.0);
		fs_brightColor = fs_color;
		gl_FragDepth = gl_FragCoord.z;
		return;
	}

	mat4 vmInv = inverse(vm);
	vec3 fragPosWs = (vmInv * vec4(intersection.xyz, 1.0)).xyz;
	vec4 colSum = calc_interpolated_color(fragPosWs); // Expensive!
	fs_color = vec4(colSum.rgb, 1.0);
	fs_brightColor = fs_color;

	if(debugMode == DEBUG_FLAT_COLOR) {
		gl_FragDepth = gl_FragCoord.z;
		return;
	}

#if LIGHTING_ENABLED == 1
	if(is_particle_unlit(u_instance.renderFlags) == false) {
		vec3 fragNormal = -calc_implicit_area_normal(vm, intersection);

		// Calculate Model Matrix
		vec3 fragNormalWs = normalize((vmInv * vec4(fragNormal.xyz, 0.0)).xyz);

#if ENABLE_BLOB_TEXTURE == 1
		/*if(u_instance.reflectionIntensity > 0)
		{
			vec3 r = refract(fragPosWs -u_renderSettings.posCam.xyz,fragNormalWs,u_instance.refractionIndexRatio);
			vec4 texCol = texture(u_blobTexture,r);
			fs_color.rgb = mix(fs_color.rgb,texCol.rgb,u_instance.reflectionIntensity);
		}*/
#endif

		vec4 albedoColor = vec4(1, 1, 1, 1);
		vec4 instanceColor = vec4(1, 1, 1, 1);
		vec4 rma = vec4(1, 1, 1, 1);

		PbrMaterial pbrMat;
		pbrMat.color = vec4(get_mat_color_factor(), get_mat_alpha_factor()) *fs_color;
		pbrMat.roughnessFactor = get_mat_roughness_factor();
		pbrMat.metalnessFactor = get_mat_metalness_factor();
		pbrMat.aoFactor = get_mat_ao_factor();
		pbrMat.alphaMode = get_mat_alpha_mode();
		pbrMat.alphaCutoff = get_mat_alpha_cutoff();
		uint flags = get_mat_flags();

		MaterialInfo materialInfo = build_material_info(pbrMat, flags, albedoColor, instanceColor, rma, 0.0, vec2(0, 0));

		vec3 view = normalize(u_renderSettings.posCam.xyz - fragPosWs);
		vec3 normal = fragNormalWs;

		ivec2 location = ivec2(gl_FragCoord.xy);
		uint tileSize = get_tile_size();
		ivec2 tileID = location / ivec2(tileSize, tileSize);
		uint index = tileID.y * get_number_of_tiles_x() + tileID.x;
		uint tileStartOffset = index * MAX_SCENE_LIGHTS;
		vec3 color = vec3(0, 0, 0);
		color.rgb = get_ibl_contribution(materialInfo, normal, view, 1.0 /* reflectionProbeIntensity */);
		color.rgb += calc_pbr_direct_lighting(materialInfo, normal, view, fragPosWs, false /* areShadowsEnabled */);
		color = clamp(color, 0.0, 1.0);
		fs_color.rgb = color;
		fs_brightColor = fs_color;
	}
#endif
	gl_FragDepth = calc_frag_depth(get_projection_matrix(), vec4(intersection, 1.0));
}
