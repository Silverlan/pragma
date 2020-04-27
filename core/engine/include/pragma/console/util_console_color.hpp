/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __UTIL_CONSOLE_COLOR_HPP__
#define __UTIL_CONSOLE_COLOR_HPP__

#include <mathutil/umath.h>
#include <mathutil/color.h>
#include <optional>

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
	DLLENGINE ConsoleColorFlags get_active_console_color_flags();
	DLLENGINE std::optional<Color> console_color_flags_to_color(ConsoleColorFlags flags);
	DLLENGINE ConsoleColorFlags color_to_console_color_flags(const Color &color);
};
REGISTER_BASIC_BITWISE_OPERATORS(util::ConsoleColorFlags);

#endif
