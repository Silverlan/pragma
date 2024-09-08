#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(PREPASS, NORMAL_MAP)) uniform sampler2D u_normalBuffer;
layout(LAYOUT_ID(PREPASS, DEPTH_BUFFER)) uniform sampler2D u_depthBuffer;
layout(LAYOUT_ID(NOISE, RANDOM_NORMAL)) uniform sampler2D u_noiseTexture;

#include "/common/inputs/camera.glsl"
#include "/common/inputs/render_settings.glsl"
#include "/functions/position_from_depth.glsl"
#include "/functions/fs_linearize_depth.glsl"

const int kernelSize = 64;
layout(std140, LAYOUT_ID(SAMPLE, BUFFER)) uniform SampleBuffer { vec4 samples[kernelSize]; }
u_sampleBuffer;

layout(location = 0) out float fs_occlusion;

const float radius = 40;
const float bias = 1; //0.025;

layout(LAYOUT_PUSH_CONSTANTS()) uniform pushConstants { uvec2 renderTargetDimensions; }
u_pushConstants;

// Source: https://learnopengl.com/Advanced-Lighting/SSAO
void main()
{
	vec2 noiseScale = vec2(u_pushConstants.renderTargetDimensions.x, u_pushConstants.renderTargetDimensions.y) / 4.0;

	float d = texture(u_depthBuffer, vs_vert_uv).r;
	mat4 invVp = inverse(u_camera.VP);
	vec3 fragPos = (u_camera.V * vec4(calc_position_from_depth(d, vs_vert_uv, invVp), 1.0)).xyz;

	vec3 normal = texture(u_normalBuffer, vs_vert_uv).rgb;
	vec3 randomVec = texture(u_noiseTexture, vs_vert_uv * noiseScale).xyz;

	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for(uint i = 0; i < kernelSize; ++i) {
		vec3 sampleValue = TBN * vec3(u_sampleBuffer.samples[i].xyz);
		sampleValue = fragPos + sampleValue * radius;

		vec4 offset = vec4(sampleValue, 1.0);
		offset = u_camera.P * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		float d = texture(u_depthBuffer, offset.xy).r;
		vec3 sampleXyz = calc_position_from_depth(d, vs_vert_uv, invVp);
		float sampleDepth = (u_camera.V * vec4(sampleXyz, 1.0)).z;
		occlusion += (sampleDepth >= sampleValue.z + bias ? 1.0 : 0.0);

		//float rangeCheck = smoothstep(0.0,1.0,radius /abs(fragPos.z -sampleDepth));
		float rangeCheck = smoothstep(0.0, 1.0, length(fragPos - sampleXyz) / radius);
		rangeCheck = 1.0 - rangeCheck;
		occlusion += (sampleDepth >= sampleValue.z + bias ? 1.0 : 0.0) * rangeCheck;
	}
	occlusion = 1.0 - (occlusion / kernelSize);
	fs_occlusion = pow(occlusion, 0.5);
}
