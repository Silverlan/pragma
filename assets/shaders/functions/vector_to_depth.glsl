#ifndef F_VECTOR_TO_DEPTH_GLS
#define F_VECTOR_TO_DEPTH_GLS

float vector_to_depth(vec3 v, float f)
{
	vec3 vecAbs = abs(v);
	float localZComp = max(vecAbs.x, max(vecAbs.y, vecAbs.z));

	const float n = 2.0; // Near Plane

	float normZComp = (f + n) / (f - n) - (2.0 * f * n) / (f - n) / localZComp;
	return (normZComp + 1.0) * 0.5;
}

#endif
