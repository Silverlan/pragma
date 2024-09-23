#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "/math/geometry.glsl"

#define MAX_GRADIENT_NODES 3 // Push constants musn't exceed 128 bytes (8 +8 +16 +(16 +16) *MAX_GRADIENT_NODES = 128)

layout(location = 0) in vec2 vs_vert_uv;

struct Node {
	vec4 color;   // 16 Bytes
	float offset; // 4 Bytes + 12 Bytes alignment
};

layout(LAYOUT_PUSH_CONSTANTS()) uniform pushConstants
{
	ivec2 textureSize;              // 8 Bytes
	vec2 boxIntersection;           // 8 Bytes
	int nodeCount;                  // 4 Bytes + 12 Bytes alignment
	Node nodes[MAX_GRADIENT_NODES]; // 16 Bytes + 16 Bytes
}
u_pushConstants;

layout(location = 0) out vec4 fs_color;

void main()
{
	vec2 p = get_closest_point_on_line_to_point(-u_pushConstants.boxIntersection, u_pushConstants.boxIntersection, vec2(gl_FragCoord.x / float(u_pushConstants.textureSize.x), gl_FragCoord.y / float(u_pushConstants.textureSize.y)));
	float diameter = length(u_pushConstants.boxIntersection);
	float v = 1.0 - (distance(p, u_pushConstants.boxIntersection) / diameter);
	fs_color = vec4(0, 0, 0, 0);
	float prevOffset = 0.0;
	vec4 prevColor = vec4(0, 0, 0, 1);
	bool found = false;
	if(v >= 0.0) {
		for(int i = 0; i < u_pushConstants.nodeCount; i++) {
			vec4 nodeColor = u_pushConstants.nodes[i].color;
			float offset = u_pushConstants.nodes[i].offset;
			if(offset >= prevOffset) {
				if(v <= offset) {
					float v = (v - prevOffset) / (offset - prevOffset);
					fs_color += vec4(mix(prevColor, nodeColor, v));
					found = true;
					break;
				}
				prevOffset = offset;
				prevColor = nodeColor;
			}
		}
	}
	else // Same, but in reverse
	{
		v = 1.0 + v;
		for(int i = u_pushConstants.nodeCount; i > 0;) {
			--i;
			int offsetIdx = (u_pushConstants.nodeCount - 1) - i;
			vec4 nodeColor = u_pushConstants.nodes[i].color;
			float offset = u_pushConstants.nodes[offsetIdx].offset;
			if(offset >= prevOffset) {
				if(v <= offset) {
					float v = (v - prevOffset) / (offset - prevOffset);
					fs_color += vec4(mix(prevColor, nodeColor, v));
					found = true;
					break;
				}
				prevOffset = offset;
				prevColor = nodeColor;
			}
		}
	}
	if(found == false)
		fs_color = prevColor;
}
