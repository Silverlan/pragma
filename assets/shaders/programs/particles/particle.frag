#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

const uint CSPEC_SHADOW_QUALITY = 0;

#include "particle_mode.glsl"
#include "particle_generic.glsl"
#include "particle_instance.glsl"
#include "particle_alpha_mode.glsl"
#include "fs_particle_texture.glsl"
#include "/common/export.glsl"
#include "/common/color.glsl"
#include "/common/compare_op.glsl"
#include "/common/fs_alpha_test.glsl"
#include "/common/inputs/fs_renderer.glsl"
#include "/common/inputs/scene_depth.glsl"
#include "/common/inputs/render_settings.glsl"
#include "/lighting/inputs/light_sources.glsl"
#include "/lighting/inputs/vis_light_buffer.glsl"
#include "/common/limits.glsl"
#include "/common/light_source.glsl"
#include "/math/ease.glsl"
#include "/programs/particles/particle_vertex_data.glsl"

#if LIGHTING_ENABLED == 1
//vec3 get_vertex_position_ws() {return fs_in.vert_pos_ws.xyz;}
//vec3 get_vertex_position_cs() {return fs_in.vert_pos_cs;}
//vec3 get_vertex_normal() {return fs_in.vert_normal;}
//vec3 get_vertex_normal_cs() {return fs_in.vert_normal_cs;}
//#include "../lighting/fs_lighting.gls"
#endif

layout(location = 0) out vec4 fs_color;
layout(location = 1) out vec4 fs_brightColor;

vec3 apply_directional_light(LightSourceData light, uint lightIndex, vec3 normal, vec3 view)
{
	vec3 pointToLight = -light.direction.xyz;
	vec3 shade = vec3(0, 0, 0); //get_point_shade(pointToLight, materialInfo, normal, view);
	float shadowFactor = 0;     //get_directional_light_shadow_factor(lightIndex);
	return light.color.a * light.color.rgb * shade * shadowFactor;
}

vec3 calc_particle_lighting(vec3 normal, vec3 baseColor)
{
	vec3 color = vec3(0.0, 0.0, 0.0);

	color = baseColor.rgb;
	vec3 view = normalize(u_renderSettings.posCam.xyz - fs_in.vert_pos_ws.xyz);
#if 0
    vec3 view = normalize(u_renderSettings.posCam.xyz - fs_in.vert_pos_ws.xyz);

    bool useLightmaps = is_light_map_enabled();
    if(useLightmaps)
    {
        vec4 colLightMap = texture(u_lightMap,fs_in.vert_uv_lightmap.xy);
        //baseColor.rgb *= colLightMap.rgb;
        color = baseColor.rgb;
    }
    /*else */if(is_ibl_enabled())
       color += get_ibl_contribution(materialInfo, normal, view);

    if(useLightmaps)
    {
        vec4 colLightMap = texture(u_lightMap,fs_in.vert_uv_lightmap.xy);
        colLightMap.rgb *= u_renderSettings.lightmapIntensity;
        color.rgb = color.rgb *(u_renderSettings.lightmapSqrt *sqrt(colLightMap.rgb) +(1.0 -u_renderSettings.lightmapSqrt) *colLightMap.rgb);
    }
#endif
	ivec2 location = ivec2(gl_FragCoord.xy);
	uint tileSize = get_tile_size();
	ivec2 tileID = location / ivec2(tileSize, tileSize);
	uint index = tileID.y * get_number_of_tiles_x() + tileID.x;
	uint offset = index * MAX_SCENE_LIGHTS;
	for(uint i = 0; i < 1 && visibleLightTileIndicesBuffer.data[offset + i].index != -1; i++) {
		uint lightIndex = visibleLightTileIndicesBuffer.data[offset + i].index;
		LightSourceData light = get_light_source(lightIndex);
		float shadowFactor = 1.0;
		if((light.flags & FLIGHT_TYPE_SPOT) != 0)
			; // color += apply_spot_light(light,lightIndex,materialInfo,normal,view);
		else if((light.flags & FLIGHT_TYPE_POINT) != 0)
			; // color += apply_point_light(light,lightIndex,materialInfo,normal,view);
		else
			color += apply_directional_light(light, lightIndex, normal, view);
	}

	return color;
}

void main()
{
	uint viewportW = u_instance.viewportSize >> 16;
	uint viewportH = (u_instance.viewportSize << 16) >> 16;
	float zScene = get_scene_depth(viewportW, viewportH);
	if(zScene < gl_FragCoord.z)
		discard;
	vec2 uv = fs_in.vert_uv; //get_particle_uv(fs_in.vert_uv,fs_in.particle_start +cur_time(),0);//cur_time(),fs_in.particle_start);
	vec4 col = fs_in.particle_color;
	rgba8_to_rgba16(col);

	//fs_color = get_particle_color(uv) *col;
	// TODO
	//fs_color = get_particle_color(fs_in.particle_start);
	vec4 outColor = get_animated_texture_color(uv); //texture(u_smpParticle,uv);
	outColor.rgba *= u_instance.colorFactor;
	outColor.rgb *= col.rgb;
	//outColor.a *= col.a;
	//outColor.rgb = (outColor.rgb +col.rgb);
	//outColor.rgb /= max(outColor.r,max(outColor.g,outColor.b));
	outColor.a = get_particle_alpha(outColor);
	outColor.a *= col.a;
	//outColor.rgb *= 10.0; // TODO

	if(u_instance.orientation != PARTICLE_ORIENTATION_TYPE_WORLD && (u_instance.renderFlags & PARTICLE_FRENDER_SOFT) != 0) {
		float depthFactor = get_depth_smooth_factor(u_instance.nearZ, u_instance.farZ, zScene);

		// TODO: Looks better when rendering with additive blending, has to be confirmed for alpha-based blending
		// If it doesn't look good with alpha-based blending, just use unmodified 'depthFactor' if PARTICLE_FRENDER_ADDITIVE_BLEND_BY_COLOR flag isn't set
		depthFactor = ease_quadratic_out(depthFactor);

		if((u_instance.renderFlags & PARTICLE_FRENDER_ADDITIVE_BLEND_BY_COLOR) != 0)
			outColor.rgb *= depthFactor;
		else
			outColor.a *= depthFactor;
	}

	if((u_instance.renderFlags & PARTICLE_FRENDER_DEPTH_PASS) != 0) {
		float a = outColor.a;
		if((u_instance.renderFlags & PARTICLE_FRENDER_ADDITIVE_BLEND_BY_COLOR) != 0)
			a = max(outColor.r, max(outColor.g, outColor.b));
		// Don't write depth values if we're below a certain alpha transparency threshold
		if(a < 0.1)
			discard;
		return;
	}

	// Alpha test
	uint alphaCompareOp = COMPARE_OP_ALWAYS;
	float alphaCompareRef = 0.0;
	switch(u_instance.alphaMode) {
	case PT_ALPHA_MODE_MASKED:
		alphaCompareOp = COMPARE_OP_EQUAL;
		alphaCompareRef = 1.0;
		break;
	}
	apply_alpha_test(outColor.a, alphaCompareOp, alphaCompareRef);
	//

#if LIGHTING_ENABLED == 1
	//if(is_particle_unlit() == false)
	//	outColor.rgb = calc_particle_lighting(fs_in.vert_normal,outColor.rgb);//calculate_lighting(outColor.rgb,uv,mat4(1.0),get_vertex_position_ws(),fs_in.vert_normal,false);
#endif
	fs_color = outColor;
}
