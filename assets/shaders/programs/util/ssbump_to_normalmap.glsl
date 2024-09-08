#ifndef F_SH_SSBUMP_TO_NORMALMAP_GLS
#define F_SH_SSBUMP_TO_NORMALMAP_GLS

// Magic numbers used in the Source Engine to generate self-shadowing bump maps
#define OO_SQRT_3 0.57735025882720947
const vec3 bumpBasis0 = vec3(0.81649661064147949, 0.0, OO_SQRT_3);
const vec3 bumpBasis1 = vec3(-0.40824833512306213, 0.70710676908493042, OO_SQRT_3);
const vec3 bumpBasis2 = vec3(-0.40824821591377258, -0.7071068286895752, OO_SQRT_3);
vec3 ss_bumpmap_sample_to_tangent_space_normal(vec3 ssSample)
{
	vec3 n = normalize(bumpBasis0 * ssSample.x + bumpBasis1 * ssSample.y + bumpBasis2 * ssSample.z);
	n += 1.0;
	n /= 2.0;
	return n;
}

#endif
