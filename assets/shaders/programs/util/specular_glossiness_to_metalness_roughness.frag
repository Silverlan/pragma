#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "/common/rma.glsl"

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURES, DIFFUSE)) uniform sampler2D u_diffuseMap;
layout(LAYOUT_ID(TEXTURES, SPECULAR_GLOSSINESS)) uniform sampler2D u_specularGlossinessMap;
layout(LAYOUT_ID(TEXTURES, AMBIENT_OCCLUSION)) uniform sampler2D u_aoMap;

layout(location = 0) out vec4 fs_output;

#define FPASS_ALBEDO 0
#define FPASS_RMA 1

layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants
{
	vec4 diffuseFactor;
	vec4 specularFactor; // Alpha is glossiness factor

	uint outputMode;
}
u_pushConstants;

// Source: https://github.com/KhronosGroup/glTF/blob/master/extensions/2.0/Khronos/KHR_materials_pbrSpecularGlossiness/examples/convert-between-workflows/js/three.pbrUtilities.js

const vec3 dielectricSpecular = vec3(0.04, 0.04, 0.04);
const float epsilon = 1e-6;

float get_perceived_brightness(vec3 col) { return sqrt(0.299 * col.r * col.r + 0.587 * col.g * col.g + 0.114 * col.b * col.b); }

float get_max_component(vec3 col) { return max(col.r, max(col.g, col.b)); }

float solve_metallic(float diffuse, float specular, float oneMinusSpecularStrength)
{
	if(specular < dielectricSpecular.r)
		return 0.0;
	float a = dielectricSpecular.r;
	float b = diffuse * oneMinusSpecularStrength / (1 - dielectricSpecular.r) + specular - 2 * dielectricSpecular.r;
	float c = dielectricSpecular.r - specular;
	float D = max(b * b - 4 * a * c, 0);
	return clamp((-b + sqrt(D)) / (2 * a), 0, 1);
}

void main()
{
	vec4 diffuseValues = texture(u_diffuseMap, vs_vert_uv);
	vec3 diffuse = diffuseValues.rgb * u_pushConstants.diffuseFactor.rgb;
	float opacity = diffuseValues.a * u_pushConstants.diffuseFactor.a;

	vec4 specularValues = texture(u_specularGlossinessMap, vs_vert_uv);
	vec3 specular = specularValues.rgb * u_pushConstants.specularFactor.rgb;
	float glossiness = specularValues.a * u_pushConstants.specularFactor.a;

	float oneMinusSpecularStrength = 1 - get_max_component(specular);
	float metallic = solve_metallic(get_perceived_brightness(diffuse), get_perceived_brightness(specular), oneMinusSpecularStrength);

	vec3 baseColorFromDiffuse = diffuse * (oneMinusSpecularStrength / (1 - dielectricSpecular.r) / max(1 - metallic, epsilon));
	vec3 baseColorFromSpecular = (specular - (dielectricSpecular * (1 - metallic))) * (1 / max(metallic, epsilon));
	vec3 baseColor = clamp(mix(baseColorFromDiffuse, baseColorFromSpecular, metallic * metallic), 0, 1);

	// We need to write the output images in multiple passes because they may differ in size
	if(u_pushConstants.outputMode == FPASS_ALBEDO)
		fs_output = vec4(baseColor, opacity);
	else if(u_pushConstants.outputMode == FPASS_RMA) {
		fs_output = vec4(0, 0, 0, 1);
		fs_output[RMA_CHANNEL_AO] = texture(u_aoMap, vs_vert_uv)[RMA_CHANNEL_AO];
		fs_output[RMA_CHANNEL_METALNESS] = metallic;
		fs_output[RMA_CHANNEL_ROUGHNESS] = 1.0 - glossiness;
	}
}
