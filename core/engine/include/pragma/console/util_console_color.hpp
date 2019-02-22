#ifndef __UTIL_CONSOLE_COLOR_HPP__
#define __UTIL_CONSOLE_COLOR_HPP__

#include <mathutil/umath.h>

namespace util
{
	enum class ConsoleColorFlags : uint32_t
	{
		None = 0u,
		Red = 1u,
		Green = Red<<1u,
		Blue = Green<<1u,
		Intensity = Blue<<1u,
		
		BackgroundRed = Intensity<<1u,
		BackgroundGreen = BackgroundRed<<1u,
		BackgroundBlue = BackgroundGreen<<1u,
		BackgroundIntensity = BackgroundBlue<<1u,

		Yellow = Red | Green,
		Magenta = Red | Blue,
		Cyan = Blue | Green,
		White = Red | Green | Blue,
		Black = None,
		
		BackgroundYellow = BackgroundRed | BackgroundGreen,
		BackgroundMagenta = BackgroundRed | BackgroundBlue,
		BackgroundCyan = BackgroundBlue | BackgroundGreen,
		BackgroundWhite = BackgroundRed | BackgroundGreen | BackgroundBlue,
		BackgroundBlack = None
	};
	DLLENGINE bool set_console_color(ConsoleColorFlags flags);
	DLLENGINE bool reset_console_color();
};
REGISTER_BASIC_BITWISE_OPERATORS(util::ConsoleColorFlags);

#endif
