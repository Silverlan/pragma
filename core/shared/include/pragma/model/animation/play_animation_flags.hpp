#ifndef __PLAY_ANIMATION_FLAGS_HPP__
#define __PLAY_ANIMATION_FLAGS_HPP__

#include <cinttypes>
#include <mathutil/umath.h>

namespace pragma
{
	enum class FPlayAnim : uint32_t
	{
		None = 0u,
		Reset = 1u,
		Transmit = Reset<<1u, // Only used for entities
		SnapTo = Transmit<<1u, // Unused
		Default = Transmit
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::FPlayAnim)

#endif
