#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "/math/matrix.glsl"

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURE, NORMAL)) uniform sampler2D u_normalMap;

layout(location = 0) out vec4 fs_normal;

vec3 oct_to_float32x3(vec2 e)
{
	vec3 v = vec3(e.xy, 1.0 - abs(e.x) - abs(e.y));
	return normalize(v);
}

vec3 hemi_oct_to_tangent_normal(vec4 bumpNormal)
{
	vec2 temp = vec2(bumpNormal.x + bumpNormal.y - (256.0 / 255.0), bumpNormal.x - bumpNormal.y);
	return normalize(oct_to_float32x3(temp));
}

void main()
{
	vec4 col = textureLod(u_normalMap, vs_vert_uv, 0); // We have to grab the normal values from lod 0, unsure why

	col = vec4(col.b, col.g, col.r, col.a);
	float nx = ((col.b + col.g));
	float ny = (col.b - col.g);
	float nz = 1.0 - abs(nx) - abs(ny);

	float l = sqrt((nx * nx) + (ny * ny) + (nz * nz));
	col.a = col.r; //b to alpha
	col.b = (((nx / l * 0.5) + 0.5));
	col.g = (((ny / l * 0.5) + 0.5));
	col.r = (((nz / l * 0.5) + 0.5));

	vec4 n = vec4(col.r, col.g, col.b, col.a);
	n.xyz = hemi_oct_to_tangent_normal(n);

	mat4 m = get_rotation_matrix(vec3(0, 0, 1), -radians(45.0 * 3));
	n.xyz = (m * vec4(n.xyz, 0.0)).xyz;

	n.xyz = (n.xyz + 1.0) / 2.0;
	fs_normal = vec4(n.xyz, 1.0);
}
