#ifndef F_FS_ALPHA_TEST_GLS
#define F_FS_ALPHA_TEST_GLS

#include "compare_op.glsl"

void apply_alpha_test(float alpha, uint compareOp, float compareRef)
{
	switch(compareOp) {
	case COMPARE_OP_NEVER:
		discard;
	case COMPARE_OP_LESS:
		if(alpha < compareRef)
			break;
		discard;
	case COMPARE_OP_EQUAL:
		if(alpha == compareRef)
			break;
		discard;
	case COMPARE_OP_LESS_OR_EQUAL:
		if(alpha <= compareRef)
			break;
		discard;
	case COMPARE_OP_GREATER:
		if(alpha > compareRef)
			break;
		discard;
	case COMPARE_OP_NOT_EQUAL:
		if(alpha != compareRef)
			break;
		discard;
	case COMPARE_OP_GREATER_OR_EQUAL:
		if(alpha >= compareRef)
			break;
		discard;
	case COMPARE_OP_ALWAYS:
		break;
	}
}

#endif
