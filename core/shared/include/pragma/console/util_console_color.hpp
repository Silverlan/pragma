// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __UTIL_CONSOLE_COLOR_HPP__
#define __UTIL_CONSOLE_COLOR_HPP__

#include <mathutil/umath.h>
#include <mathutil/color.h>
#include <optional>

namespace util {
	enum class ConsoleColorFlags : uint32_t {
		None = 0u,
		Red = 1u,
		Green = Red << 1u,
		Blue = Green << 1u,
		Intensity = Blue << 1u,

		BackgroundRed = Intensity << 1u,
		BackgroundGreen = BackgroundRed << 1u,
		BackgroundBlue = BackgroundGreen << 1u,
		BackgroundIntensity = BackgroundBlue << 1u,

		Reset = BackgroundIntensity << 1u,

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
	DLLNETWORK bool set_console_color(ConsoleColorFlags flags);
	DLLNETWORK bool reset_console_color();
	DLLNETWORK ConsoleColorFlags get_active_console_color_flags();
	DLLNETWORK std::optional<Color> console_color_flags_to_color(ConsoleColorFlags flags);
	DLLNETWORK ConsoleColorFlags color_to_console_color_flags(const Color &color);
	DLLNETWORK std::string get_ansi_color_code(ConsoleColorFlags flags);

	enum class ConsoleDecoratorFlags : uint32_t {
		None = 0,
		Bold = 1,
		Underline = Bold << 1,
		SlowBlink = Underline << 1,
		Framed = SlowBlink << 1,
		Encircled = Framed << 1,
		Overlined = Encircled << 1,
		Reset = Overlined << 1,
	};

	DLLNETWORK std::string get_true_color_code(std::optional<Color> foregroundColor, std::optional<Color> backgroundColor = {}, ConsoleDecoratorFlags flags = ConsoleDecoratorFlags::None);
	DLLNETWORK std::string get_reset_color_code();
};
REGISTER_BASIC_BITWISE_OPERATORS(util::ConsoleColorFlags);
REGISTER_BASIC_BITWISE_OPERATORS(util::ConsoleDecoratorFlags);

#endif
