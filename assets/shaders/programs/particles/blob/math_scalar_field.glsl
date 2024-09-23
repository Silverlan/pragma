#ifndef F_FS_MATH_SCALAR_FIELD_GLS
#define F_FS_MATH_SCALAR_FIELD_GLS

#define USE_LINEARLY_DECLINING_SCALAR_FIELD 0

#include "particle_blob.glsl"
#include "../particle_vertex_data.glsl"
#include "fs_particle_data.glsl"

const float IMPLICIT_AREA_ISO_VALUE = 0.2;
const float EFFECTIVE_PARTICLE_RADIUS = 5.2;
const float IMPLICIT_AREA_EPSILON = 0.0006;

const uint MAX_RAY_ITERATION_STEPS = 20;
const float MAX_RAY_STEP_SIZE = 0.1;
const float RAY_STEP_SIZE = 50.0;

const uint TWO_BYTE_MASK = (1 << 16) - 1;

uint get_adjacent_blob_index(int i)
{
	switch(i) // Some GPUs / Drivers don't like dynamic array indexing
	{
	case 1:
		return fs_in.blobNeighbors[1];
	case 2:
		return fs_in.blobNeighbors[2];
	case 3:
		return fs_in.blobNeighbors[3];
	default:
		return fs_in.blobNeighbors[0];
	}
	return fs_in.blobNeighbors[0];
}

float si_linear(vec3 x, vec3 xi, float radius)
{
	float l = length(x - xi);
	if(l < radius)
		return radius - l;
	return 0.0;
}

float si(vec3 x, vec3 xi, float radius)
{
#if USE_LINEARLY_DECLINING_SCALAR_FIELD == 1
	return si_linear(x, xi, radius);
#else
	float effectiveParticleRadius = -log(IMPLICIT_AREA_ISO_VALUE) / (radius * 0.4); // TODO!!!
	return exp(-effectiveParticleRadius * length(x - xi));
	//return exp(-EFFECTIVE_PARTICLE_RADIUS *length(x -xi) /radius);
#endif
}

float calc_neighbor_scalar_field_value(mat4 matView, vec3 eyeDir, uint id)
{
	vec3 particlePosCs = (matView * vec4(u_particleData.particles[id].position.xyz, 1.0)).xyz;
	return si(eyeDir, particlePosCs, u_particleData.particles[id].position.w);
}

float calc_scalar_field_value(mat4 matView, vec3 fragmentPosCs, vec3 eyeDir, out vec3 intersection)
{
	uint stepCount = MAX_RAY_ITERATION_STEPS;
	float stepInc = RAY_STEP_SIZE;
	float currentStep = 0.0;
	float scalarFieldSum = 0.0;
	float result = 0.0;
	intersection = vec3(0.0, 0.0, 0.0);
	while(stepCount-- > 0 && stepInc > MAX_RAY_STEP_SIZE) {
		vec3 stepPos = fragmentPosCs - eyeDir * currentStep;
		scalarFieldSum = 0.0;

		for(int i = 0; i < 4; ++i) {
			uint v = get_adjacent_blob_index(i);

			uint id0 = v & TWO_BYTE_MASK;
			if(id0 == TWO_BYTE_MASK)
				break; // There are no remaining neighbors
			scalarFieldSum += calc_neighbor_scalar_field_value(matView, stepPos, id0);

			uint id1 = (v >> 16) & TWO_BYTE_MASK;
			if(id1 == TWO_BYTE_MASK)
				break; // There are no remaining neighbors
			scalarFieldSum += calc_neighbor_scalar_field_value(matView, stepPos, id1);
		}

		if(scalarFieldSum < IMPLICIT_AREA_ISO_VALUE) {
			currentStep -= stepInc;
			stepInc *= 0.5;
		}
		else {
			result = scalarFieldSum;
			intersection = stepPos;
		}
		currentStep += stepInc;
	}
	return result;
}

vec3 calc_implicit_area_normal(mat4 matView, vec3 intersectionPos)
{
	vec3 positions[6] = {// Min
	  vec3(intersectionPos.x - IMPLICIT_AREA_EPSILON, intersectionPos.y, intersectionPos.z), vec3(intersectionPos.x, intersectionPos.y - IMPLICIT_AREA_EPSILON, intersectionPos.z), vec3(intersectionPos.x, intersectionPos.y, intersectionPos.z - IMPLICIT_AREA_EPSILON),

	  // Max
	  vec3(intersectionPos.x + IMPLICIT_AREA_EPSILON, intersectionPos.y, intersectionPos.z), vec3(intersectionPos.x, intersectionPos.y + IMPLICIT_AREA_EPSILON, intersectionPos.z), vec3(intersectionPos.x, intersectionPos.y, intersectionPos.z + IMPLICIT_AREA_EPSILON)};

	vec3 gradientVector = vec3(0.0, 0.0, 0.0);
	for(int j = 0; j < 3; ++j) {
		for(int i = 0; i < 4; ++i) {
			uint v = get_adjacent_blob_index(i);

			uint id0 = v & TWO_BYTE_MASK;
			if(id0 == TWO_BYTE_MASK)
				break; // There are no remaining neighbors
			gradientVector[j] += calc_neighbor_scalar_field_value(matView, positions[j + 3], id0) - calc_neighbor_scalar_field_value(matView, positions[j], id0);

			uint id1 = (v >> 16) & TWO_BYTE_MASK;
			if(id1 == TWO_BYTE_MASK)
				break; // There are no remaining neighbors
			gradientVector[j] += calc_neighbor_scalar_field_value(matView, positions[j + 3], id1) - calc_neighbor_scalar_field_value(matView, positions[j], id1);
		}
	}
	gradientVector *= 1.0 / (2.0 * IMPLICIT_AREA_EPSILON);
	return normalize(gradientVector);
}

#endif
