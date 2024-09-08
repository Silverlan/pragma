#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(SCENE, TEXTURE)) uniform sampler2D u_texture;
layout(LAYOUT_ID(VELOCITY, TEXTURE)) uniform sampler2D u_textureVelocity;

layout(location = 0) out vec4 fs_color;

layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants
{
	float velocityScale;
	uint blurQuality;
}
u_pushConstants;

vec3 apply_blur(vec3 velocity, vec2 texCoord, vec3 color, const uint numSamples)
{
	float factor = 20.0;
	vec2 dt = (velocity.xy / numSamples) * factor;
	for(int i = 1; i < numSamples; ++i, texCoord += dt) {
		// Sample the color buffer along the velocity vector.
		vec3 currentColor = texture(u_texture, texCoord).rgb;
		// Add the current color to our color sum.
		color += currentColor;
	} // Average all of the samples to get the final blur color.
	return color / numSamples;
}

void main()
{
	vec4 color = texture(u_texture, vs_vert_uv);
	vec3 velocity = texture(u_textureVelocity, vs_vert_uv).xyz;

	float uVelocityScale = u_pushConstants.velocityScale;
	velocity.xy = velocity.xy * uVelocityScale;

	// Get the initial color at this pixel.
	vec2 texCoord = vs_vert_uv;
	if(u_pushConstants.blurQuality == 0)
		color.rgb = apply_blur(velocity, texCoord, color.rgb, 20);
	else if(u_pushConstants.blurQuality == 1)
		color.rgb = apply_blur(velocity, texCoord, color.rgb, 80);
	else if(u_pushConstants.blurQuality == 2)
		color.rgb = apply_blur(velocity, texCoord, color.rgb, 200);
	fs_color = color;
}
