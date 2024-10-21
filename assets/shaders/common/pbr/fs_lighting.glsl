#ifndef F_FS_PBR_LIGHTING_GLS
#define F_FS_PBR_LIGHTING_GLS

#include "/common/pbr/material.glsl"
#include "/common/inputs/fs_renderer.glsl"
#include "/common/inputs/entity.glsl"
#include "/common/inputs/fs_lightmap.glsl"
#include "/common/inputs/textures/normal_map.glsl"
#include "/common/inputs/textures/ssao_map.glsl"
#include "/programs/scene/scene_push_constants.glsl"
#include "lighting/lighting_direct.glsl"
#include "lighting/ibl.glsl"
#include "lighting/light_spot_shadow.glsl"
#include "lighting/light_point.glsl"
#include "lighting/light_directional.glsl"

vec3 apply_lighting(LightSourceData light, uint lightIndex, MaterialInfo materialInfo, vec3 normal, vec3 view)
{
	bool enableShadows = are_shadows_enabled();
	return ((light.flags & FLIGHT_TYPE_SPOT) == 0) ? apply_spot_light(light, lightIndex, materialInfo, normal, view, get_vertex_position_ws(), enableShadows) : apply_point_light(light, lightIndex, materialInfo, normal, view, get_vertex_position_ws(), enableShadows);
}

vec3 calc_pbr_lighting(vec2 uv, MaterialInfo materialInfo, uint materialFlags, vec4 baseColor)
{
	//baseColor = vec4(0,0,0,1);
	vec3 color = vec3(0.0, 0.0, 0.0);

	vec3 normal = get_normal_from_map(uv, materialFlags);
	normal = normalize((get_model_matrix() * vec4(normal.xyz, 0.0)).xyz);

	vec3 view = normalize(u_renderSettings.posCam.xyz - get_vertex_position_ws());

	bool useLightmaps = false;
	if(CSPEC_ENABLE_LIGHT_MAPS == 1) {
		useLightmaps = is_light_map_enabled();
		//if(useLightmaps)
		//	color = baseColor.rgb;
	}
	if(CSPEC_ENABLE_IBL == 1) {
		/*else */ //if(is_ibl_enabled())
		if(!useLightmaps)
			color += get_ibl_contribution(materialInfo, normal, view, get_reflection_probe_intensity());
	}

	if(CSPEC_ENABLE_LIGHT_MAPS == 1) {
		// TODO: Lightmap mode should be determined by specialization constant to avoid if-condition overhead
		if(useLightmaps) {
			vec2 uv = get_vertex_uv_lightmap();
			vec4 colDirect = texture(u_lightMap, uv);
			float exposure = get_lightmap_exposure_pow();
			if(is_indirect_light_map_enabled()) {
				vec3 colIndirect = texture(u_lightMapIndirect, uv).rgb;
				if(is_directional_light_map_enabled()) {
					vec3 dominantDir = texture(u_lightMapDominant, uv).rgb;
					dominantDir = dominantDir * 2.0 - 1.0;
					dominantDir = normalize(dominantDir);

					vec3 shade = get_point_shade_lm(-dominantDir, materialInfo, normal, view, colDirect.rgb * exposure);
					color = baseColor.rgb * (shade + colIndirect.rgb * exposure);
				}
				else
					color = baseColor.rgb * ((colDirect.rgb + colIndirect.rgb) * exposure);
			}
			else
				color.rgb += baseColor.rgb * (exposure * colDirect.rgb);
		}
	}

	if(CSPEC_ENABLE_LIGHT_SOURCES == 1 && CSPEC_ENABLE_DYNAMIC_LIGHTING == 1)
		color += calc_pbr_direct_lighting(materialInfo, normal, view, get_vertex_position_ws(), are_shadows_enabled());
	return color;
}

#endif
