#ifndef __RENDER_COMPONENT_FLAGS_HPP__
#define __RENDER_COMPONENT_FLAGS_HPP__

#include <mathutil/umath.h>

namespace pragma
{
	enum class FRenderFlags : uint32_t
	{
		None = 0u,
		CastShadows = 1u,
		Unlit = CastShadows<<1u
	};
	REGISTER_BASIC_BITWISE_OPERATORS(FRenderFlags);
};

#endif
