#ifndef __C_SHADOW_TYPE_HPP__
#define __C_SHADOW_TYPE_HPP__

#include <cinttypes>
#include <mathutil/umath.h>

enum class ShadowType : uint8_t
{
	None = 0,
	StaticOnly = 1,
	Full = StaticOnly | 2
};
REGISTER_BASIC_ARITHMETIC_OPERATORS(ShadowType);

#endif
