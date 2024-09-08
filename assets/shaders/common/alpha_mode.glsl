#ifndef F_FS_ALPHA_MODE_GLS
#define F_FS_ALPHA_MODE_GLS

#include "conditionals.glsl"

#define ALPHA_MODE_OPAQUE 0
#define ALPHA_MODE_MASK 1
#define ALPHA_MODE_BLEND 2

float apply_alpha_mode(float a, uint alphaMode, float alphaCutoff)
{
	// if(alphaMode == ALPHA_MODE_OPAQUE) a = 1.0
	a += (1.0 - a) * when_eq(alphaMode, ALPHA_MODE_OPAQUE);

	a = when_neq(alphaMode, ALPHA_MODE_MASK) * a +                       // If alpha mode is not masked, just use alpha
	  and(when_eq(alphaMode, ALPHA_MODE_MASK), when_ge(a, alphaCutoff)); // Otherwise use (a >= alphaCutoff) ? 1.0 : 0.0
	return a;
}

#endif
