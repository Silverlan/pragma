#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define DEBUG_MODE DEBUG_MODE_NONE

#include "eye.glsl"
#include "/programs/scene/pbr/fs_core.glsl"

void main()
{
	vec2 uvBase = get_vertex_uv();
	vec2 uv = uvBase;

	vec4 irisProjectionU = u_pushConstants.irisProjectionU;
	vec4 irisProjectionV = u_pushConstants.irisProjectionV;

	vec3 worldPos = get_vertex_position_ws();
	vec4 worldPosProjZ;
	worldPosProjZ.xyz = worldPos.xyz;

	// Transform into projection space
	mat4 cViewProj = get_view_projection_matrix();
	vec4 vProjPos = cViewProj * vec4(worldPos, 1.0);
	worldPosProjZ.w = vProjPos.z;

	vec2 vCorneaUv;
	vCorneaUv.x = dot(irisProjectionU, vec4(worldPosProjZ.xyz, 1.0));
	vCorneaUv.y = dot(irisProjectionV, vec4(worldPosProjZ.xyz, 1.0));
	vec2 vSphereUv = vCorneaUv;

	// Dilation
	float maxDilationFactor = u_pushConstants.maxDilationFactor;
	float dilationFactor = u_pushConstants.dilationFactor;

	float irisUvRadius = u_pushConstants.irisUvRadius;
	float fPupilCenterToBorder = clamp(length(vSphereUv.xy) / irisUvRadius, 0.0, 1.0);
	float fPupilDilateFactor = dilationFactor;
	vSphereUv.xy *= mix(1.0, fPupilCenterToBorder, clamp(fPupilDilateFactor, 0.0, maxDilationFactor) * 2.5 - 1.25);

	vSphereUv += 1.0;
	vSphereUv /= 2.0;

	uv = vSphereUv;

	vec2 irisUvClampRange = u_pushConstants.eyeOrigin.xy;
	vec4 irisColor = fetch_iris_map(clamp(vSphereUv, irisUvClampRange.x, irisUvClampRange.y));
	vec4 scleraColor = fetch_sclera_map(uvBase);
	vec4 albedoColor = mix(scleraColor, irisColor, irisColor.a);
	fs_color = calc_pbr(albedoColor, uv, u_pushConstants.debugMode);
	if(CSPEC_BLOOM_OUTPUT_ENABLED == 1)
		extract_bright_color(fs_color);
}
