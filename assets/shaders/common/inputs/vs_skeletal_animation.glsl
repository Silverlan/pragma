#ifndef F_VS_WEIGHTED_GLS
#define F_VS_WEIGHTED_GLS

#include "/common/vertex_outputs/vs_vertex_data_locations.glsl"
#include "/common/limits.glsl"

layout(location = SHADER_BONE_WEIGHT_ID_LOCATION) in ivec4 in_boneWeightIDs;
layout(location = SHADER_BONE_WEIGHT_LOCATION) in vec4 in_weights;

layout(location = SHADER_BONE_WEIGHT_EXT_ID_LOCATION) in ivec4 in_boneWeightExtIDs;
layout(location = SHADER_BONE_WEIGHT_EXT_LOCATION) in vec4 in_weightsExt;

//uniform bool u_weighted; // Defined in push constants
layout(std140, LAYOUT_ID(INSTANCE, BONE_MATRICES)) uniform Bones { mat4 matrices[MAX_BONES]; }
u_bones;

mat4 calc_bone_matrix(bool weighted, bool weightedExt)
{
	if(weighted == false)
		return mat4(1.0);
	mat4 mat = mat4(0.0);
	for(int i = 0; i < 4; i++) {
		if(in_boneWeightIDs[i] != -1)
			mat += in_weights[i] * u_bones.matrices[in_boneWeightIDs[i]];
	}
	if(weightedExt) {
		for(int i = 4; i < 8; i++) {
			if(in_boneWeightExtIDs[i] != -1)
				mat += in_weightsExt[i] * u_bones.matrices[in_boneWeightExtIDs[i]];
		}
	}
	return mat;
}

vec4 get_weighted_vertex_position(bool weighted, vec4 vertexPos)
{
	mat4 mBone = calc_bone_matrix(weighted, false);
	vec4 vpos = mBone * vertexPos;
	return vpos;
}
vec4 get_weighted_vertex_position(vec4 vertexPos) { return get_weighted_vertex_position(true, vertexPos); }

#endif
