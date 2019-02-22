#ifndef __SNAPSHOT_FLAGS_HPP__
#define __SNAPSHOT_FLAGS_HPP__

#include <mathutil/umath.h>

namespace pragma
{
	enum class SnapshotFlags : uint8_t
	{
		None = 0u,
		PhysicsData = 1u,
		ComponentData = PhysicsData<<1u
	};
	REGISTER_BASIC_BITWISE_OPERATORS(SnapshotFlags);
};

#endif
