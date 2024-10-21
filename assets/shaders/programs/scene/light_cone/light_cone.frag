#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "light_cone.glsl"
#include "/common/export.glsl"
#include "/common/inputs/camera.glsl"
#include "/common/inputs/render_settings.glsl"
#include "/common/inputs/scene_depth.glsl"
#include "/common/pixel_outputs/fs_bloom_color.glsl"

layout(LAYOUT_ID(TEXTURE, TEXTURE)) uniform sampler2D u_texture;

// See http://john-chapman-graphics.blogspot.com/2013/01/good-enough-volumetrics-for-spotlights.html

#include "/functions/fs_linearize_depth.glsl"

void main()
{
	vec3 origin = u_pushConstants.coneOrigin.xyz;
	float coneLength = u_pushConstants.coneOrigin.w;
	uint viewportW = u_pushConstants.resolution >> 16;
	uint viewportH = (u_pushConstants.resolution << 16) >> 16;
	float nearZ = u_pushConstants.nearZ;
	float farZ = u_pushConstants.farZ;

	vec4 instanceColor = u_pushConstants.color;
	fs_color = instanceColor;
	float intensity = fs_color.a;

	fs_color.a = 1.0 - min(length(fs_in.vert_pos_ws - origin) / coneLength, 1.0);

	mat4 vm = get_view_matrix();
	vec3 vertPosCs = normalize((vm * vec4(fs_in.vert_pos_ws.xyz, 1.0)).xyz);
	vec3 vertNormCs = fs_in.vert_normal_cs;

	float dt = min(dot(normalize(vertNormCs), normalize(vertPosCs)), 0);

	dt = pow(dt, 4);
	fs_color.a *= dt;
	fs_color.a *= get_depth_smooth_factor(nearZ, farZ, get_scene_depth(viewportW, viewportH));

	vec3 camDir = normalize(fs_in.vert_pos_ws - u_renderSettings.posCam.xyz);
	vec3 camDirCs = normalize((vm * vec4(camDir, 0.0)).xyz);
	float l = 1.0 - max(0.0, dot(fs_in.vert_normal_cs, camDirCs));

	// TODO: Re-implement shadows
	/*int lightIndex = int(u_pushConstants.boundLightIndex);
	if(lightIndex != -1)
	{
		LightSourceData light = get_light_source(lightIndex);
		float shadow = 1.0;
		if(light.shadowMapIndexStatic > 0)
			shadow = get_spot_light_shadow_factor(lightIndex,false);
		if(light.shadowMapIndexDynamic > 0)
			shadow = min(shadow,get_spot_light_shadow_factor(lightIndex,true));
		if(shadow < 1.0)
		{
			vec4 depthCoord = get_light_vertex_position(lightIndex);
			depthCoord.z *= light.position.w; // Undo transformation from get_light_vertex_position()
			depthCoord = depthCoord /light.position.w;
			
			fs_color.a *= 1.0 -(1.0 -shadow) *0.8;
		}
	}*/
	fs_color.a *= intensity;

	fs_brightColor.rgb = fs_color.rgb * fs_color.a;
	fs_brightColor.a = 1.0;

	fs_color.a = 0; // Rendering of the main mesh is disabled for now, since just the bloom generally looks better
}
