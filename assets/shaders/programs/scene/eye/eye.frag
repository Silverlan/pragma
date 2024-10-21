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

	vec3 vertPos = get_vertex_position_ws();
#if ENABLE_NOISE == 1
	vec3 eyeOrigin = u_pushConstants.eyeOrigin.xyz;
	vec3 eyePos = eyeOrigin;
	vec3 nWorld = normalize(vertPos - eyeOrigin.xyz);

	vec3 worldViewVector = normalize(vertPos - eyePos.xyz);

	vec3 eyeSocketUp = normalize(-u_pushConstants.irisProjectionV.xyz);
	vec3 eyeSocketLeft = normalize(-u_pushConstants.irisProjectionU.xyz);
	vec3 vWorldTangent = normalize(cross(eyeSocketUp.xyz, nWorld.xyz));
	vec3 vWorldBinormal = normalize(cross(nWorld.xyz, vWorldTangent.xyz));

	{
		vec3 Q1 = dFdx(vertPos);
		vec3 Q2 = dFdy(vertPos);
		vec2 st1 = dFdx(uv);
		vec2 st2 = dFdy(uv);

		vec3 N = normalize(get_vertex_normal());
		vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
		vec3 B = -normalize(cross(N, T));
		//nWorld = N;
		//vWorldTangent = T;
		//vWorldBinormal = B;
	}
	vec3 viewVecTangent = world_to_tangent_normalized(worldViewVector.xyz, nWorld.xyz, vWorldTangent.xyz, vWorldBinormal.xyz);

#endif

	vec3 worldPos = vertPos; // TODO: This needs to be unmodified (no matrix transform!)
	vec4 worldPosProjZ;
	worldPosProjZ.xyz = worldPos.xyz;

	// Transform into projection space
	mat4 cViewProj = get_view_projection_matrix();
	vec4 vProjPos = cViewProj * vec4(worldPos, 1.0);

	worldPosProjZ.w = vProjPos.z;
	worldPosProjZ.xyz = vertPos;

	vec2 vCorneaUv; // Note: Cornea texture is a cropped version of the iris texture
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

#if ENABLE_NOISE == 1
	float irisOffset = texture(u_parallaxMap, vSphereUv.xy).b;
	float parallaxStrength = 1.0;
	vec2 vParallax = ((viewVecTangent.xz * irisOffset * parallaxStrength) / (1.0 - viewVecTangent.y));
	vParallax.x = -vParallax.x;

	vec2 uvCorneaNoise = vSphereUv.xy + (vParallax.xy * 0.5);
	vec4 colCorneaNoise = texture(u_glowMap, uvCorneaNoise.xy);
	float corneaNoise = colCorneaNoise.r * texture(u_albedoMap, vSphereUv).a;
#endif

	uv = vSphereUv;

	vec2 texCoords = uv; // TODO: apply_parallax(use_parallax_map(),uv);
	vec4 albedoColor = texture(u_albedoMap, texCoords);
	fs_color = calc_pbr(albedoColor, texCoords, u_pushConstants.debugMode);
	if(CSPEC_BLOOM_OUTPUT_ENABLED == 1)
		extract_bright_color(fs_color);
}
