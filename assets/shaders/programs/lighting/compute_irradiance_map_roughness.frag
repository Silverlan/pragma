#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "/common/pbr/core.glsl"

struct VertexData {
	vec3 worldPos;
};
layout(location = 0) in VertexData fs_in;

layout(LAYOUT_ID(IRRADIANCE, IRRADIANCE)) uniform samplerCube u_cubemap;
layout(std140, LAYOUT_ID(ROUGHNESS, ROUGHNESS)) uniform RoughnessData
{
	float roughness;
	float resolution;
}
u_roughnessData;

layout(location = 0) out vec4 fs_color;

// Source: https://learnopengl.com
void main()
{
	vec3 N = normalize(fs_in.worldPos);

	// make the simplyfying assumption that V equals R equals the normal
	vec3 R = N;
	vec3 V = R;

	const uint SAMPLE_COUNT = 1024u;
	vec3 prefilteredColor = vec3(0.0);
	float totalWeight = 0.0;

	for(uint i = 0u; i < SAMPLE_COUNT; ++i) {
		// generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
		vec2 Xi = hammersley(i, SAMPLE_COUNT);
		vec3 H = importance_sample_ggx(Xi, N, u_roughnessData.roughness);
		vec3 L = normalize(2.0 * dot(V, H) * H - V);

		float NdotL = max(dot(N, L), 0.0);
		if(NdotL > 0.0) {
			// sample from the environment's mip level based on roughness/pdf
			float D = distribution_ggx(N, H, u_roughnessData.roughness);
			float NdotH = max(dot(N, H), 0.0);
			float HdotV = max(dot(H, V), 0.0);
			float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

			float resolution = u_roughnessData.resolution; // resolution of source cubemap (per face)
			float saTexel = 4.0 * M_PI / (6.0 * resolution * resolution);
			float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

			float mipLevel = u_roughnessData.roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

			prefilteredColor += textureLod(u_cubemap, L, mipLevel).rgb * NdotL;
			totalWeight += NdotL;
		}
	}

	prefilteredColor = prefilteredColor / totalWeight;

	fs_color = vec4(prefilteredColor, 1.0);
}
