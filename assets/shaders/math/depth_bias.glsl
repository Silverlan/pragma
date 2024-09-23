#ifndef F_DEPTH_BIAS_GLS
#define F_DEPTH_BIAS_GLS

float calc_projection_depth_bias_offset(float p, float nearZ, float farZ, float d, float delta)
{
	// See http://mathfor3dgameprogramming.com/ , chapter 9.1.1
	// delta = Eye space z offset
	float pz = -d; // Negative eye space depth value

	float epsilon = -2.0 * farZ * nearZ * delta / ((farZ + nearZ) * pz * (pz + delta));
	return p * (1.0 + epsilon);
}

// Does *not* work for some reason
// void apply_projection_depth_bias_offset(inout mat4 p,float nearZ,float farZ,float d,float delta) {p[2][2] = calc_projection_depth_bias_offset(p[2][2],nearZ,farZ,d,delta);}

#define apply_projection_depth_bias_offset(p, nearZ, farZ, d, delta) p[2][2] = calc_projection_depth_bias_offset(p[2][2], nearZ, farZ, d, delta);

#endif
