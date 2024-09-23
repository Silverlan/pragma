#ifndef F_FS_PBR_LIGHTING_GLS
#define F_FS_PBR_LIGHTING_GLS

#include "/modules/sh_pbr.gls"

vec3 pbr_calc_light_radiance(uint lightIndex, vec3 fragWorldPos, vec3 N, vec3 V, vec3 F0, vec3 albedo, float roughness, float metalness)
{
	LightSourceData light = get_light_source(lightIndex);

	vec3 lightPos = light.position.xyz;
	vec3 lightColor = light.color.rgb;
	vec3 radiance = lightColor * light.color.a;

	if((light.flags & FLIGHT_TYPE_SPOT) != 0)
		; //radiance *= calc_spot_light_attenuation(lightIndex);
	else if((light.flags & FLIGHT_TYPE_POINT) != 0)
		radiance *= calc_point_light_attenuation(lightIndex, fragWorldPos);
	else
		; //radiance *= calc_directional_light_attenuation(lightIndex);

	// calculate per-light radiance
	vec3 L = normalize(lightPos - fragWorldPos);
	vec3 H = normalize(V + L);

	// Cook-Torrance BRDF
	float NDF = distribution_ggx(N, H, roughness);
	float G = geometry_smith(N, V, L, roughness);
	vec3 F = fresnel_schlick(max(dot(H, V), 0.0), F0);

	vec3 nominator = NDF * G * F;
	float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
	vec3 specular = nominator / denominator;

	// kS is equal to Fresnel
	vec3 kS = F;
	// for energy conservation, the diffuse and specular light can't
	// be above 1.0 (unless the surface emits light); to preserve this
	// relationship the diffuse component (kD) should equal 1.0 - kS.
	vec3 kD = vec3(1.0) - kS;
	// multiply kD by the inverse metalness such that only non-metals
	// have diffuse lighting, or a linear blend if partly metal (pure metals
	// have no diffuse light).
	kD *= 1.0 - metalness;

	// scale light by NdotL
	float NdotL = max(dot(N, L), 0.0);

	// add to outgoing radiance Lo
	return (kD * albedo / M_PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
}

#endif
