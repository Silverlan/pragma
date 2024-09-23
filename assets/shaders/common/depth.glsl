#ifndef F_DEPTH_GLS
#define F_DEPTH_GLS

#include "/functions/fs_linearize_depth.glsl"

float get_depth_smooth_factor(float nearZ, float farZ, float zScene, float sc)
{
	float zSceneLinear = get_linearized_depth(zScene, nearZ, farZ);
	float depthLinear = get_linearized_depth(gl_FragCoord.z, nearZ, farZ);
	float d = zSceneLinear - depthLinear;
	return min(d * sc, 1.0);
}

float get_depth_smooth_factor(float nearZ, float farZ, float zScene) { return get_depth_smooth_factor(nearZ, farZ, zScene, 4000); }

float calc_frag_depth(mat4 projMat, vec4 fragPosVs, float minDepth, float maxDepth)
{
	// Source: https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/
	// https://vulkan.lunarg.com/doc/view/1.0.30.0/windows/vkspec.chunked/ch23s05.html
	vec4 fragPosCs = projMat * fragPosVs;
	float zd = fragPosCs.z / fragPosCs.w;
	float pz = maxDepth - minDepth;
	return (pz * zd) + minDepth;
}
float calc_frag_depth(mat4 projMat, vec4 fragPosVs) { return calc_frag_depth(projMat, fragPosVs, 0.0, 1.0); }

// Deprecated, OpenGL only!
/*
float calc_frag_depth(mat4 projMat,vec4 fragPosVs,float nearZ,float farZ)
{
	vec4 fragPosClipSpace = projMat *fragPosVs;
	float ndcDepth = fragPosClipSpace.z /fragPosClipSpace.w;
	return (((farZ -nearZ) *ndcDepth) +nearZ +farZ) /2.0;
}
float calc_frag_depth(mat4 projMat,vec4 fragPosVs) {return calc_frag_depth(projMat,fragPosVs,0.0,1.0);}
float calc_frag_depth(mat4 viewMat,mat4 projMat,vec3 fragPosWs,float nearZ,float farZ) {return calc_frag_depth(projMat,viewMat *vec4(fragPosWs,1.0),nearZ,farZ);}
float calc_frag_depth(mat4 viewMat,mat4 projMat,vec3 fragPosWs) {return calc_frag_depth(viewMat,projMat,fragPosWs,0.0,1.0);}
*/

#endif
