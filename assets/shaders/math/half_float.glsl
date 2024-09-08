#ifndef F_HALF_FLOAT_GLS
#define F_HALF_FLOAT_GLS

float float16_to_float(int fltInt16)
{
	int fltInt32 = ((fltInt16 & 0x8000) << 16);
	fltInt32 |= ((fltInt16 & 0x7fff) << 13) + 0x38000000;
	return intBitsToFloat(fltInt32);
}

#endif
