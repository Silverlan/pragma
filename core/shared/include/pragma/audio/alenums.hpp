#ifndef __ALENUMS_HPP__
#define __ALENUMS_HPP__

#include "pragma/networkdefinitions.h"
#include <mathutil/umath.h>

struct DLLNETWORK SoundCacheInfo
{
	SoundCacheInfo()
		: duration(0.f),mono(false),stereo(false)
	{}
	float duration;
	bool mono;
	bool stereo;
};

enum class ALChannel : uint32_t
{
	Auto = 0,
	Mono,
	Both
};

enum class ALCreateFlags : uint32_t
{
	None = 0,
	Mono = 1,
	Stream = Mono<<1,
	DontTransmit = Stream<<1 // Serverside only
};
REGISTER_BASIC_BITWISE_OPERATORS(ALCreateFlags);

#endif
