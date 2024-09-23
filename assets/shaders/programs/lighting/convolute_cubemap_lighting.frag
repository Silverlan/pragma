#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "/math/math.glsl"

struct VertexData {
	vec3 worldPos;
};
layout(location = 0) in VertexData fs_in;

layout(LAYOUT_ID(TEXTURE, CUBEMAP)) uniform samplerCube u_cubemap;

layout(location = 0) out vec4 fs_color;

// Source: https://learnopengl.com
void main()
{
	// The world vector acts as the normal of a tangent surface
	// from the origin, aligned to WorldPos. Given this normal, calculate all
	// incoming radiance of the environment. The result of this radiance
	// is the radiance of light coming from -Normal direction, which is what
	// we use in the PBR shader to sample irradiance.
	vec3 N = normalize(fs_in.worldPos);

	vec3 irradiance = vec3(0.0);

	// tangent space calculation from origin point
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = cross(up, N);
	up = cross(N, right);

	float sampleDelta = 0.025;
	float nrSamples = 0.0;
	for(float phi = 0.0; phi < 2.0 * M_PI; phi += sampleDelta) {
		for(float theta = 0.0; theta < 0.5 * M_PI; theta += sampleDelta) {
			// spherical to cartesian (in tangent space)
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			// tangent space to world
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

			irradiance += texture(u_cubemap, sampleVec).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	}
	irradiance = M_PI * irradiance * (1.0 / float(nrSamples));

	fs_color = vec4(irradiance, 1.0);
}
