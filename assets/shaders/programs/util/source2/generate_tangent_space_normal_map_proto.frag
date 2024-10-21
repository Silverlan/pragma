#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURE, NORMAL)) uniform sampler2D u_normalMap;

layout(location = 0) out vec4 fs_normal;

void main()
{
	// SteamVR and Dota 2
	vec4 bumpNormal = texture(u_normalMap, vs_vert_uv);
	vec2 temp = vec2(bumpNormal.w, bumpNormal.y) * 2 - 1;
	vec3 tangentNormal = vec3(temp, sqrt(1 - temp.x * temp.x - temp.y * temp.y));
	tangentNormal.y = -tangentNormal.y;
	tangentNormal = (tangentNormal + 1.0) / 2.0;
	fs_normal = vec4(tangentNormal.xyz, 1.0);
}
