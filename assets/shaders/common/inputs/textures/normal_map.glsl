#ifndef F_SH_NORMALMAPPING_GLS
#define F_SH_NORMALMAPPING_GLS

#ifdef MATERIAL_NORMAL_MAP_ENABLED
#include "/common/vertex_outputs/tangentspace.glsl"
#include "/common/material_flags.glsl"
#endif

#ifdef GLS_FRAGMENT_SHADER
#include "/lighting/inputs/fs_light_sources.glsl"
#if ENABLE_TANGENT_SPACE_NORMALS == 1
vec3 calc_tangent_space_normal(vec2 uv)
{
	vec3 ncol = fetch_normal_map(uv).rgb;
	vec3 nmap;
	float normalMapScale = 1.0;
	nmap.xy = (2.0 * ncol.rg - 1.0) * normalMapScale;
	nmap.z = sqrt(1.0 - dot(nmap.xy, nmap.xy));

	return normalize((fs_in.vert_tangent * nmap.x) + (fs_in.vert_bitangent * nmap.y) + (get_vertex_normal() * nmap.z));
}
#endif
float calculate_normal_light_direction_angle(uint lightIdx, vec2 uv, uint materialFlags)
{
	float cosTheta;
	LightSourceData light = get_light_source(lightIdx);
#ifdef MATERIAL_NORMAL_MAP_ENABLED
	if(use_normal_map(materialFlags) == true) {
		vec3 ntex_ts = normalize(fetch_normal_map(uv).rgb * 2.0 - 1.0);
		vec3 n = ntex_ts;
		vec3 l = normalize(get_light_direction_ts(lightIdx));
		cosTheta = clamp(dot(n, l), 0.0, 1.0);
	}
	else {
		vec3 n = get_vertex_normal_cs();
		vec3 l = normalize(get_light_direction_cs(lightIdx));
		cosTheta = clamp(dot(n, l), 0.0, 1.0);
	}
#endif
	return cosTheta;
}
vec3 get_normal_from_map(vec2 texCoords, uint materialFlags)
{
#ifdef MATERIAL_NORMAL_MAP_ENABLED
	if(use_normal_map(materialFlags)) {
		vec3 tangentNormal = fetch_normal_map(texCoords).xyz * 2.0 - 1.0;

		vec3 vertPos = get_vertex_position_ws();
		vec3 Q1 = dFdx(vertPos);
		vec3 Q2 = dFdy(vertPos);
		vec2 st1 = dFdx(texCoords);
		vec2 st2 = dFdy(texCoords);

		vec3 N = normalize(get_vertex_normal());
		vec3 T = Q1 * st2.t - Q2 * st1.t;
		T = normalize((get_model_matrix() * vec4(T, 0)).xyz);
		vec3 B = -normalize(cross(N, T));
		mat3 TBN = mat3(T, B, N); // TODO: use fs_in.TBN;

		return normalize(TBN * tangentNormal);
	}
#endif
	return get_vertex_normal();
}
#endif

#endif
