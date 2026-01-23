// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#ifdef _WIN32
#include <Windows.h>
#endif

module pragma.shared;

import :console.enums;

#undef max

static pragma::console::ConsoleColorFlags s_activeConsoleColorFlags = pragma::console::ConsoleColorFlags::None;
std::string pragma::console::get_ansi_color_code(ConsoleColorFlags flags)
{
	if(math::is_flag_set(flags, ConsoleColorFlags::Reset))
		return "\u001b[0m";
	auto colorCode = 0u;
	auto colorFlags = flags & ConsoleColorFlags::White;
	auto bIntensity = (flags & ConsoleColorFlags::Intensity) != ConsoleColorFlags::None;
	std::string strColorCode;
	if(colorFlags != ConsoleColorFlags::None) {
		switch(colorFlags) {
		case ConsoleColorFlags::Red:
			colorCode = bIntensity ? 91 : 31;
			break;
		case ConsoleColorFlags::Green:
			colorCode = bIntensity ? 92 : 32;
			break;
		case ConsoleColorFlags::Blue:
			colorCode = bIntensity ? 94 : 34;
			break;
		case ConsoleColorFlags::Yellow:
			colorCode = bIntensity ? 93 : 33;
			break;
		case ConsoleColorFlags::Magenta:
			colorCode = bIntensity ? 95 : 35;
			break;
		case ConsoleColorFlags::Cyan:
			colorCode = bIntensity ? 96 : 36;
			break;
		case ConsoleColorFlags::White:
			colorCode = bIntensity ? 97 : 37;
			break;
		case ConsoleColorFlags::Black:
			colorCode = bIntensity ? 90 : 30;
			break;
		}
		strColorCode = "\033[" + std::to_string(colorCode) + "m";
	}

	colorCode = 0u;
	colorFlags = flags & ConsoleColorFlags::BackgroundWhite;
	bIntensity = (flags & ConsoleColorFlags::BackgroundIntensity) != ConsoleColorFlags::None;
	if(colorFlags != ConsoleColorFlags::None) {
		switch(colorFlags) {
		case ConsoleColorFlags::BackgroundRed:
			colorCode = bIntensity ? 101 : 41;
			break;
		case ConsoleColorFlags::BackgroundGreen:
			colorCode = bIntensity ? 102 : 42;
			break;
		case ConsoleColorFlags::BackgroundBlue:
			colorCode = bIntensity ? 104 : 44;
			break;
		case ConsoleColorFlags::BackgroundYellow:
			colorCode = bIntensity ? 103 : 43;
			break;
		case ConsoleColorFlags::BackgroundMagenta:
			colorCode = bIntensity ? 105 : 45;
			break;
		case ConsoleColorFlags::BackgroundCyan:
			colorCode = bIntensity ? 106 : 46;
			break;
		case ConsoleColorFlags::BackgroundWhite:
			colorCode = bIntensity ? 107 : 47;
			break;
		case ConsoleColorFlags::BackgroundBlack:
			colorCode = bIntensity ? 100 : 40;
			break;
		}
		strColorCode += "\033[" + std::to_string(colorCode) + "m"; // Background color
	}
	return strColorCode;
}
bool pragma::console::set_console_color(ConsoleColorFlags flags)
{
	reset_console_color();
	s_activeConsoleColorFlags = flags;
#ifdef _WIN32
	auto hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if(!hOut)
		return false;
	WORD wflags {};
	if((flags & pragma::console::ConsoleColorFlags::Red) != pragma::console::ConsoleColorFlags::None)
		wflags |= FOREGROUND_RED;
	if((flags & pragma::console::ConsoleColorFlags::Green) != pragma::console::ConsoleColorFlags::None)
		wflags |= FOREGROUND_GREEN;
	if((flags & pragma::console::ConsoleColorFlags::Blue) != pragma::console::ConsoleColorFlags::None)
		wflags |= FOREGROUND_BLUE;
	if((flags & pragma::console::ConsoleColorFlags::Intensity) != pragma::console::ConsoleColorFlags::None)
		wflags |= FOREGROUND_INTENSITY;

	if((flags & pragma::console::ConsoleColorFlags::BackgroundRed) != pragma::console::ConsoleColorFlags::None)
		wflags |= BACKGROUND_RED;
	if((flags & pragma::console::ConsoleColorFlags::BackgroundGreen) != pragma::console::ConsoleColorFlags::None)
		wflags |= BACKGROUND_GREEN;
	if((flags & pragma::console::ConsoleColorFlags::BackgroundBlue) != pragma::console::ConsoleColorFlags::None)
		wflags |= BACKGROUND_BLUE;
	if((flags & pragma::console::ConsoleColorFlags::BackgroundIntensity) != pragma::console::ConsoleColorFlags::None)
		wflags |= BACKGROUND_INTENSITY;
	return static_cast<bool>(SetConsoleTextAttribute(hOut, wflags));
#else
	std::cout << get_ansi_color_code(flags);
#endif
	return true;
}
bool pragma::console::reset_console_color()
{
	std::cout.flush();
	s_activeConsoleColorFlags = ConsoleColorFlags::None;
#ifdef _WIN32
	auto hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if(!hOut)
		return false;
	if(static_cast<bool>(SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)) == false)
		return false;
#else
	std::cout << "\033[0m";
#endif
	return true;
}

pragma::console::ConsoleColorFlags pragma::console::get_active_console_color_flags() { return s_activeConsoleColorFlags; }

const std::unordered_map<pragma::console::ConsoleColorFlags, Color> colorMap = {{pragma::console::ConsoleColorFlags::Red | pragma::console::ConsoleColorFlags::Intensity, Color {209, 17, 65}},
  {pragma::console::ConsoleColorFlags::Green | pragma::console::ConsoleColorFlags::Intensity, Color {0, 177, 89}}, {pragma::console::ConsoleColorFlags::Blue | pragma::console::ConsoleColorFlags::Intensity, Color {0, 174, 219}},
  {pragma::console::ConsoleColorFlags::Yellow | pragma::console::ConsoleColorFlags::Intensity, Color {255, 196, 37}}, {pragma::console::ConsoleColorFlags::Cyan | pragma::console::ConsoleColorFlags::Intensity, Color {132, 193, 255}},
  {pragma::console::ConsoleColorFlags::Magenta | pragma::console::ConsoleColorFlags::Intensity, Color {255, 51, 119}}, {pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity, Color {255, 255, 255}},

  {pragma::console::ConsoleColorFlags::Red, Color {209 / 2, 17 / 2, 65 / 2}}, {pragma::console::ConsoleColorFlags::Green, Color {0 / 2, 177 / 2, 89 / 2}}, {pragma::console::ConsoleColorFlags::Blue, Color {0 / 2, 174 / 2, 219 / 2}},
  {pragma::console::ConsoleColorFlags::Yellow, Color {255 / 2, 196 / 2, 37 / 2}}, {pragma::console::ConsoleColorFlags::Cyan, Color {132 / 2, 193 / 2, 255 / 2}}, {pragma::console::ConsoleColorFlags::Magenta, Color {255 / 2, 51 / 2, 119 / 2}},
  {pragma::console::ConsoleColorFlags::White, Color {255 / 2, 255 / 2, 255 / 2}}};
std::optional<Color> pragma::console::console_color_flags_to_color(ConsoleColorFlags flags)
{
	auto foregroundFlags = flags & (ConsoleColorFlags::White | ConsoleColorFlags::Intensity);
	auto it = colorMap.find(foregroundFlags);
	if(it != colorMap.end())
		return it->second;
	return {};
}

pragma::console::ConsoleColorFlags pragma::console::color_to_console_color_flags(const Color &color)
{
	auto smallestDiff = std::numeric_limits<uint16_t>::max();
	auto bestCandidate = ConsoleColorFlags::None;
	for(auto &pair : colorMap) {
		auto &colOther = pair.second;
		auto diff = static_cast<uint16_t>(math::abs(color.r - colOther.r) + math::abs(color.g - colOther.g) + math::abs(color.b - colOther.b));
		if(diff > smallestDiff)
			continue;
		smallestDiff = diff;
		bestCandidate = pair.first;
	}
	return bestCandidate;
}

std::string pragma::console::get_true_color_code(std::optional<Color> foregroundColor, std::optional<Color> backgroundColor, ConsoleDecoratorFlags flags)
{
	std::string colorCode;
	if(foregroundColor)
		colorCode += "\033[38;2;" + std::to_string(foregroundColor->r) + ";" + std::to_string(foregroundColor->g) + ";" + std::to_string(foregroundColor->b) + "m";
	if(backgroundColor)
		colorCode += "\033[48;2;" + std::to_string(backgroundColor->r) + ";" + std::to_string(backgroundColor->g) + ";" + std::to_string(backgroundColor->b) + "m";

	if(flags != ConsoleDecoratorFlags::None) {
		std::string decoratorCode;
		if((flags & ConsoleDecoratorFlags::Bold) != ConsoleDecoratorFlags::None)
			decoratorCode += ";1";
		if((flags & ConsoleDecoratorFlags::Underline) != ConsoleDecoratorFlags::None)
			decoratorCode += ";4";
		if((flags & ConsoleDecoratorFlags::SlowBlink) != ConsoleDecoratorFlags::None)
			decoratorCode += ";5";
		if((flags & ConsoleDecoratorFlags::Framed) != ConsoleDecoratorFlags::None)
			decoratorCode += ";51";
		if((flags & ConsoleDecoratorFlags::Encircled) != ConsoleDecoratorFlags::None)
			decoratorCode += ";52";
		if((flags & ConsoleDecoratorFlags::Overlined) != ConsoleDecoratorFlags::None)
			decoratorCode += ";53";
		if((flags & ConsoleDecoratorFlags::Reset) != ConsoleDecoratorFlags::None)
			decoratorCode += ";0";
		if(decoratorCode.empty() == false) {
			decoratorCode = decoratorCode.substr(1);
			colorCode += "\033[" + decoratorCode + "m";
		}
	}
	return colorCode;
}
std::string pragma::console::get_reset_color_code() { return "\u001b[0m"; }
