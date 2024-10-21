#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "/functions/fs_linearize_depth.glsl"

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURE, DEPTH_MAP)) uniform sampler2D u_depthTexture;

layout(LAYOUT_PUSH_CONSTANTS()) uniform Depth
{
	float near;
	float far;
	float contrastFactor;
}
u_depth;

layout(location = 0) out vec4 fs_color;

float linearize_depth(float d)
{
	//return u_depth.near * u_depth.far / (u_depth.far + d * (u_depth.near - u_depth.far));
	return (2.0 * d) / (u_depth.far - d * (u_depth.far - u_depth.near));
}

float linearDepth(float depthSample)
{
	depthSample = 2.0 * depthSample - 1.0;
	float zLinear = 2.0 * u_depth.near * u_depth.far / (u_depth.far + u_depth.near - depthSample * (u_depth.far - u_depth.near));
	return zLinear;
}

/*layout(LAYOUT_ID(TEXTURE, DEPTH_MAP)) uniform usampler2D u_depthTexture;
void fetch_stencil_value()
{
	uint d = texelFetch(u_depthTexture,ivec2(vs_vert_uv.x *1920,vs_vert_uv.y *1080),0).r;
	
}*/

void main()
{
	float d = get_linearized_depth(u_depthTexture, vs_vert_uv, u_depth.near, u_depth.far);
	float l = get_depth_distance(texture(u_depthTexture, vs_vert_uv).r, u_depth.near, u_depth.far);

	float l2 = u_depth.near + d * (u_depth.far - u_depth.near);
	float d2 = linearize_depth(texture(u_depthTexture, vs_vert_uv).r);
	float d3 = linearDepth(texture(u_depthTexture, vs_vert_uv).r);
	float l3 = u_depth.near + d2 * (u_depth.far - u_depth.near);
	float l4 = u_depth.near + d3 * (u_depth.far - u_depth.near);

	d = pow(d, u_depth.contrastFactor);
	fs_color = vec4(d, d, d, 1.0);
}