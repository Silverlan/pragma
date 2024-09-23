#ifndef F_SH_VALIDATE_GLS
#define F_SH_VALIDATE_GLS

#define VALIDATE_EPSILON 0.001

bool validate(float a, float b) { return abs(a - b) < VALIDATE_EPSILON; }
bool validate(vec3 a, vec3 b) { return abs(a.x - b.x) < VALIDATE_EPSILON && abs(a.y - b.y) < VALIDATE_EPSILON && abs(a.z - b.z) < VALIDATE_EPSILON; }
bool validate(mat4 a, mat4 b)
{
	return abs(a[0][0] - b[0][0]) < VALIDATE_EPSILON && abs(a[0][1] - b[0][1]) < VALIDATE_EPSILON && abs(a[0][2] - b[0][2]) < VALIDATE_EPSILON && abs(a[0][3] - b[0][3]) < VALIDATE_EPSILON && abs(a[1][0] - b[1][0]) < VALIDATE_EPSILON && abs(a[1][1] - b[1][1]) < VALIDATE_EPSILON
	  && abs(a[1][2] - b[1][2]) < VALIDATE_EPSILON && abs(a[1][3] - b[1][3]) < VALIDATE_EPSILON && abs(a[2][0] - b[2][0]) < VALIDATE_EPSILON && abs(a[2][1] - b[2][1]) < VALIDATE_EPSILON && abs(a[2][2] - b[2][2]) < VALIDATE_EPSILON && abs(a[2][3] - b[2][3]) < VALIDATE_EPSILON
	  && abs(a[3][0] - b[3][0]) < VALIDATE_EPSILON && abs(a[3][1] - b[3][1]) < VALIDATE_EPSILON && abs(a[3][2] - b[3][2]) < VALIDATE_EPSILON && abs(a[3][3] - b[3][3]) < VALIDATE_EPSILON;
}

#endif
