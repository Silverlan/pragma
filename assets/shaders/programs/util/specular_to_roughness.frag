#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURE, TEXTURE)) uniform sampler2D u_specularMap;

layout(location = 0) out vec4 fs_color;

void main()
{
	vec3 specular = texture(u_specularMap, vs_vert_uv).rgb;
	fs_color = vec4(1.0 - specular.r, 1.0 - specular.g, 1.0 - specular.b, 1.0);
}
