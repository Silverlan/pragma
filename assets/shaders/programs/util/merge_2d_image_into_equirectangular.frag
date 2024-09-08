#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "/math/math.glsl"

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURE, TEXTURE)) uniform sampler2D u_texture;
layout(LAYOUT_ID(TEXTURE_2D, TEXTURE_2D)) uniform sampler2D u_texture2;

layout(location = 0) out vec4 fs_color;

layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants
{
	float xFactor;
	uint cubeFace;
}
u_pushConstants;

const int POSITIVE_X = 0;
const int NEGATIVE_X = 1;
const int POSITIVE_Y = 2;
const int NEGATIVE_Y = 3;
const int POSITIVE_Z = 4;
const int NEGATIVE_Z = 5;

int getCubeFace(vec3 direction)
{
	vec3 absDirection = abs(direction);
	float maxComponent = max(max(absDirection.x, absDirection.y), absDirection.z);

	if(maxComponent == absDirection.x) {
		return direction.x >= 0.0 ? POSITIVE_X : NEGATIVE_X;
	}
	else if(maxComponent == absDirection.y) {
		return direction.y >= 0.0 ? POSITIVE_Y : NEGATIVE_Y;
	}
	else {
		return direction.z >= 0.0 ? POSITIVE_Z : NEGATIVE_Z;
	}
}

vec2 map3DTo2D(vec3 direction, int face)
{
	vec3 absDirection = abs(direction);

	vec2 texCoord;

	// Determine the face of the cube map
	switch(face) {
	case POSITIVE_X:
		texCoord = vec2(-direction.z, direction.y) / absDirection.x;
		break;
	case NEGATIVE_X:
		texCoord = vec2(direction.z, direction.y) / absDirection.x;
		break;
	case POSITIVE_Y:
		texCoord = vec2(direction.x, -direction.z) / absDirection.y;
		break;
	case NEGATIVE_Y:
		texCoord = vec2(direction.x, direction.z) / absDirection.y;
		break;
	case POSITIVE_Z:
		texCoord = vec2(direction.x, direction.y) / absDirection.z;
		break;
	case NEGATIVE_Z:
		texCoord = vec2(-direction.x, direction.y) / absDirection.z;
		break;
	}

	// Normalize the 2D coordinates to the range [0, 1]
	texCoord = texCoord * 0.5 + 0.5;

	return texCoord;
}

// Source: https://stackoverflow.com/a/35050222/2482983
void main()
{
	vec2 uv = vs_vert_uv * 2.0 - vec2(1.0, 1.0);

	// Convert to (lat, lon) angle
	vec2 a = (uv * vec2(M_PI * u_pushConstants.xFactor, M_PI_2)) - vec2(M_PI / 2, 0);
	// Convert to cartesian coordinates
	vec2 c = cos(a);
	vec2 s = sin(a);
	vec3 p = vec3(-s.x * c.y, -s.y, c.x * c.y);
	int cubeFace = getCubeFace(p);
	fs_color = texture(u_texture, vs_vert_uv);
	if(cubeFace == u_pushConstants.cubeFace) {
		vec4 col2 = texture(u_texture2, map3DTo2D(p, cubeFace)).rgba;

		float alpha = col2.a;
		fs_color = mix(fs_color, col2, alpha);
	}
}
