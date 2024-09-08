#ifndef F_LINEARIZE_DEPTH_GLS
#define F_LINEARIZE_DEPTH_GLS

#include "fs_texture.glsl"

float get_depth_distance(float d, float nearPlane, float farPlane)
{
	d = 2.0 * d - 1.0;
	return 2.0 * nearPlane * farPlane / (farPlane + nearPlane - d * (farPlane - nearPlane));
}
#ifdef GLS_FRAGMENT_SHADER
float get_depth_distance(float nearPlane, float farPlane) { return get_depth_distance(gl_FragCoord.z, nearPlane, farPlane); }
#endif

float get_linearized_depth(float depth, float nearPlane, float farPlane) { return (2.0 * depth) / (farPlane - depth * (farPlane - nearPlane)); }

float get_linearized_depth(sampler2D depthTexture, vec2 uv, float nearPlane, float farPlane)
{
	float z = texture(depthTexture, uv).x;
	return get_linearized_depth(z, nearPlane, farPlane);
}

float get_linearized_depth(sampler2DArray depthTexture, vec3 uv, float nearPlane, float farPlane)
{
	float z = texture(depthTexture, uv).x;
	return (2.0 * z) / (farPlane - z * (farPlane - nearPlane));
}

float get_linearized_depth(sampler2DArray depthTexture, int layer, vec2 uv, float nearPlane, float farPlane)
{
	return texture(depthTexture, vec3(uv.xy, layer)).x; //get_linearized_depth(depthTexture,vec3(uv.xy,layer),nearPlane,farPlane);
}

float get_linearized_depth(samplerCube depthTexture, int cubeSide, vec2 uv, float nearPlane, float farPlane)
{
	float uvx = uv.x * 2.0;
	float uvy = uv.y * 2.0;
	float r;
	switch(cubeSide) {
	case 0:
		{
			float z = uvx - 1.0;
			float y = uvy - 1.0;
			r = texture(depthTexture, vec3(1.0, -y, z)).x;
			break;
		}
	case 1:
		{
			float z = 1.0 - uvx;
			float y = uvy - 1.0;
			r = texture(depthTexture, vec3(-1.0, -y, z)).x;
			break;
		}
	case 2:
		{
			float x = uvx - 1.0;
			float z = 1.0 - uvy;
			r = texture(depthTexture, vec3(x, 1.0, z)).x;
			break;
		}
	case 3:
		{
			float x = uvx - 1.0;
			float z = uvy - 1.0;
			r = texture(depthTexture, vec3(x, -1.0, z)).x;
			break;
		}
	case 4:
		{
			float x = uvx - 1.0;
			float y = uvy - 1.0;
			r = texture(depthTexture, vec3(x, -y, -1.0)).x;
			break;
		}
	case 5:
		{
			float x = 1.0 - (uv.x / 4.0) * 8.0;
			float y = (uv.y / 3.0) * 6.0 - 1.0;
			r = texture(depthTexture, vec3(x, -y, 1.0)).x;
			break;
		}
	}
	return (2.0 * r) / (farPlane - r * (farPlane - nearPlane));
}

#endif
