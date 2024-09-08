#ifndef F_FS_PBR_LIGHTING_ANGULAR_INFO_GLS
#define F_FS_PBR_LIGHTING_ANGULAR_INFO_GLS

struct AngularInfo {
	float NdotL; // cos angle between normal and light direction
	float NdotV; // cos angle between normal and view direction
	float NdotH; // cos angle between normal and half vector
	float LdotH; // cos angle between light direction and half vector

	float VdotH; // cos angle between view direction and half vector

	vec3 padding;
};

AngularInfo get_angular_info(vec3 pointToLight, vec3 normal, vec3 view)
{
	// Standard one-letter names
	vec3 n = normal;           // Outward direction of surface point
	vec3 v = view;             // Direction from surface point to view
	vec3 l = pointToLight;     // Direction from surface point to light
	vec3 h = normalize(l + v); // Direction of the vector between l and v

	float NdotL = clamp(dot(n, l), 0.0, 1.0);
	float NdotV = clamp(dot(n, v), 0.0, 1.0);
	float NdotH = clamp(dot(n, h), 0.0, 1.0);
	float LdotH = clamp(dot(l, h), 0.0, 1.0);
	float VdotH = clamp(dot(v, h), 0.0, 1.0);

	return AngularInfo(NdotL, NdotV, NdotH, LdotH, VdotH, vec3(0, 0, 0));
}

#endif
