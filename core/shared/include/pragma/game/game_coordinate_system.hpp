#ifndef __GAME_COORDINATE_SYSTEM_HPP__
#define __GAME_COORDINATE_SYSTEM_HPP__

#include <cinttypes>

namespace pragma
{
	enum class Axis : uint8_t
	{
		X = 0u,
		Y,
		Z
	};
	enum class RotationOrder : uint16_t
	{
		XYZ = 0u,
		YXZ,
		XZX,
		XYX,
		YXY,
		YZY,
		ZYZ,
		ZXZ,
		XZY,
		YZX,
		ZYX,
		ZXY
	};
};

#endif
