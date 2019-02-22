#ifndef __UTIL_GAME_HPP__
#define __UTIL_GAME_HPP__

namespace util
{
	constexpr inline double units_to_metres(double units) {return units *0.025;} // 1.75 (Avg. Height in meters) /72 (Player Height in units) ~= 0.025
	constexpr inline double metres_to_units(double meters) {return meters /0.025;}
};

#endif
