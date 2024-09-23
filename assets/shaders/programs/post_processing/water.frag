#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#ifndef DESCRIPTOR_SET_MATERIAL
#define DESCRIPTOR_SET_MATERIAL 0
#endif
#ifndef DESCRIPTOR_SET_MATERIAL_BINDING_SCENE_MAP
#define DESCRIPTOR_SET_MATERIAL_BINDING_SCENE_MAP 0
#define DESCRIPTOR_SET_MATERIAL_BINDING_REFRACTION_MAP (DESCRIPTOR_SET_MATERIAL_BINDING_SCENE_MAP + 1)
#endif

#ifndef DESCRIPTOR_SET_DEPTH
#define DESCRIPTOR_SET_DEPTH (DESCRIPTOR_SET_MATERIAL + 1)
#endif
#ifndef DESCRIPTOR_SET_DEPTH_BINDING_MAP
#define DESCRIPTOR_SET_DEPTH_BINDING_MAP 0
#endif

#define DESCRIPTOR_SET_SCENE 3
#define DESCRIPTOR_SET_RENDER_SETTINGS 4
#define SHADER_UNIFORM_FOG_SET 5
#define SHADER_UNIFORM_FOG_BINDING 0

layout(LAYOUT_ID(TEXTURE, TEXTURE)) uniform sampler2D u_texture;
layout(LAYOUT_ID(DEPTH_BUFFER, MAP)) uniform sampler2D u_depthBuffer;
layout(LAYOUT_ID(REFRACTION, MAP)) uniform sampler2D u_refractionMap;

layout(location = 0) in vec2 vs_vert_uv;

layout(location = 0) out vec4 fs_color;

layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants { vec4 clipPlane; }
u_pushConstants;

#include "/common/inputs/fs_fog.glsl"
#include "/common/inputs/camera.glsl"
#include "/common/inputs/time.glsl"
#include "/common/inputs/render_settings.glsl"
#include "/functions/position_from_depth.glsl"
#include "/functions/fs_linearize_depth.glsl"

void main()
{
	float d = texture(u_depthBuffer, vs_vert_uv).r;
	mat4 invVp = inverse(u_camera.VP);
	vec3 fragPosWs = calc_position_from_depth(d, vs_vert_uv, invVp);

	vec4 clipPlane = u_pushConstants.clipPlane;
	if(dot(vec4(fragPosWs.xyz, 1.0), vec4(clipPlane.xyz, -clipPlane.w - 0.1)) > gl_ClipDistance[0]) {
		// We don't want fog above water surface
		fs_color = texture(u_texture, vs_vert_uv);
		return;
	}

	vec3 fragPosScreenWs = calc_position_from_depth(gl_FragCoord.z, vs_vert_uv, invVp);
	if(dot(vec4(fragPosScreenWs.xyz, 1.0), vec4(clipPlane.xyz, -clipPlane.w)) > gl_ClipDistance[0]) {
		// Don't render fog below water surface if we're looking at the surface from above (if the camera is clipping the water plane).
		// In this case the fog has already been rendered by the water shader.
		fs_color = texture(u_texture, vs_vert_uv);
		return;
	}

	// Apply screen refraction
	vec2 refractUv = vs_vert_uv + vec2(u_time.realTime, u_time.realTime) * 0.02;

	vec4 refractCol = texture(u_refractionMap, refractUv);
	fs_color = texture(u_texture, vs_vert_uv + refractCol.xy * 0.01);

	float depthDist = get_depth_distance(d, u_renderSettings.nearZ, u_renderSettings.farZ) * 2.0;
	fs_color.rgb = calculate_fog(fs_color.rgb, depthDist);
}
