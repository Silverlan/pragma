#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "/common/pbr/core.glsl"

layout(location = 0) in vec2 vs_vert_uv;

layout(location = 0) out vec2 fs_color;

// Source: https://learnopengl.com
vec2 integrate_brdf(float NdotV, float roughness)
{
	vec3 V;
	V.x = sqrt(1.0 - NdotV * NdotV);
	V.y = 0.0;
	V.z = NdotV;

	float A = 0.0;
	float B = 0.0;

	vec3 N = vec3(0.0, 0.0, 1.0);

	const uint SAMPLE_COUNT = 1024u;
	for(uint i = 0u; i < SAMPLE_COUNT; ++i) {
		// generates a sample vector that's biased towards the
		// preferred alignment direction (importance sampling).
		vec2 Xi = hammersley(i, SAMPLE_COUNT);
		vec3 H = importance_sample_ggx(Xi, N, roughness);
		vec3 L = normalize(2.0 * dot(V, H) * H - V);

		float NdotL = max(L.z, 0.0);
		float NdotH = max(H.z, 0.0);
		float VdotH = max(dot(V, H), 0.0);

		if(NdotL > 0.0) {
			float G = geometry_smith_ibl(N, V, L, roughness);
			float G_Vis = (G * VdotH) / (NdotH * NdotV);
			float Fc = pow(1.0 - VdotH, 5.0);

			A += (1.0 - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}
	A /= float(SAMPLE_COUNT);
	B /= float(SAMPLE_COUNT);
	return vec2(A, B);
}

void main() { fs_color = integrate_brdf(vs_vert_uv.x, 1.0 - vs_vert_uv.y); }
