#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURE, TEXTURE)) uniform sampler2D u_texture;

layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants
{
	ivec4 iparam;
	vec4 fparam;
}
u_pushConstants;

layout(location = 0) out vec4 fs_color;

#define FILTER_BICUBIC 0
#define FILTER_LANCZOS 1
layout(constant_id = 0) const uint CSPEC_FILTER = FILTER_BICUBIC;

// See https://stackoverflow.com/a/42179924/2482983
// from http://www.java-gaming.org/index.php?topic=35123.0
vec4 cubic(float v)
{
	vec4 n = vec4(1.0, 2.0, 3.0, 4.0) - v;
	vec4 s = n * n * n;
	float x = s.x;
	float y = s.y - 4.0 * s.x;
	float z = s.z - 4.0 * s.y + 6.0 * s.x;
	float w = 6.0 - x - y - z;
	return vec4(x, y, z, w) * (1.0 / 6.0);
}

vec4 textureBicubic(sampler2D tex, vec2 texCoords)
{
	vec2 texSize = textureSize(tex, 0);
	vec2 invTexSize = 1.0 / texSize;

	texCoords = texCoords * texSize - 0.5;

	vec2 fxy = fract(texCoords);
	texCoords -= fxy;

	vec4 xcubic = cubic(fxy.x);
	vec4 ycubic = cubic(fxy.y);

	vec4 c = texCoords.xxyy + vec2(-0.5, +1.5).xyxy;

	vec4 s = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
	vec4 offset = c + vec4(xcubic.yw, ycubic.yw) / s;

	offset *= invTexSize.xxyy;

	vec4 sample0 = texture(tex, offset.xz);
	vec4 sample1 = texture(tex, offset.yz);
	vec4 sample2 = texture(tex, offset.xw);
	vec4 sample3 = texture(tex, offset.yw);

	float sx = s.x / (s.x + s.y);
	float sy = s.z / (s.z + s.w);

	return mix(mix(sample3, sample2, sx), mix(sample1, sample0, sx), sy);
}

vec4 lanczosFilter(sampler2D tex, vec2 texCoords, vec4 kernel)
{
	// Get centre location
	vec2 pos = texCoords;

	// Init value
	vec4 color1 = vec4(0.0, 0.0, 0.0, 0.0);

	// Init step size in tex coords
	vec2 shape = textureSize(tex, 0);
	float dx = 1.0 / shape.x;
	float dy = 1.0 / shape.y;

	// Convolve
	int sze = 3;
	for(int y = -sze; y < sze + 1; y++) {
		for(int x = -sze; x < sze + 1; x++) {
			float k = kernel[int(abs(float(x)))] * kernel[int(abs(float(y)))];
			vec2 dpos = vec2(float(x) * dx, float(y) * dy);
			color1 += texture(tex, pos + dpos) * k;
		}
	}

	color1.a = 1.0;
	return color1;
}

void main()
{
	if(CSPEC_FILTER == FILTER_BICUBIC)
		fs_color = texture(u_texture, vs_vert_uv);
	else if(CSPEC_FILTER == FILTER_LANCZOS)
		fs_color = lanczosFilter(u_texture, vs_vert_uv, u_pushConstants.fparam);
}
