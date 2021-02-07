/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/console/util_console_color.hpp"

static util::ConsoleColorFlags s_activeConsoleColorFlags = util::ConsoleColorFlags::None;
bool util::set_console_color(ConsoleColorFlags flags)
{
	reset_console_color();
	s_activeConsoleColorFlags = flags;
#ifdef _WIN32
	auto hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if(!hOut)
		return false;
	WORD wflags {};
	if((flags &ConsoleColorFlags::Red) != ConsoleColorFlags::None)
		wflags |= FOREGROUND_RED;
	if((flags &ConsoleColorFlags::Green) != ConsoleColorFlags::None)
		wflags |= FOREGROUND_GREEN;
	if((flags &ConsoleColorFlags::Blue) != ConsoleColorFlags::None)
		wflags |= FOREGROUND_BLUE;
	if((flags &ConsoleColorFlags::Intensity) != ConsoleColorFlags::None)
		wflags |= FOREGROUND_INTENSITY;

	if((flags &ConsoleColorFlags::BackgroundRed) != ConsoleColorFlags::None)
		wflags |= BACKGROUND_RED;
	if((flags &ConsoleColorFlags::BackgroundGreen) != ConsoleColorFlags::None)
		wflags |= BACKGROUND_GREEN;
	if((flags &ConsoleColorFlags::BackgroundBlue) != ConsoleColorFlags::None)
		wflags |= BACKGROUND_BLUE;
	if((flags &ConsoleColorFlags::BackgroundIntensity) != ConsoleColorFlags::None)
		wflags |= BACKGROUND_INTENSITY;
	return static_cast<bool>(SetConsoleTextAttribute(hOut,wflags));
#else
	auto colorCode = 0u;
	auto colorFlags = flags &ConsoleColorFlags::White;
	auto bIntensity = (flags &ConsoleColorFlags::Intensity) != ConsoleColorFlags::None;
	switch(colorFlags)
	{
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
	std::cout<<"\033["<<colorCode<<"m"; // Foreground color

	colorCode = 0u;
	colorFlags = flags &ConsoleColorFlags::BackgroundWhite;
	bIntensity = (flags &ConsoleColorFlags::BackgroundIntensity) != ConsoleColorFlags::None;
	switch(colorFlags)
	{
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
	std::cout<<"\033["<<colorCode<<"m"; // Background color
#endif
	return true;
}
bool util::reset_console_color()
{
	std::cout.flush();
	s_activeConsoleColorFlags = util::ConsoleColorFlags::None;
#ifdef _WIN32
	auto hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if(!hOut)
		return false;
	if(static_cast<bool>(SetConsoleTextAttribute(hOut,FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)) == false)
		return false;
#else
	std::cout<<"\033[0m";
#endif
	return true;
}

util::ConsoleColorFlags util::get_active_console_color_flags() {return s_activeConsoleColorFlags;}

const std::unordered_map<util::ConsoleColorFlags,Color> colorMap = {
	{util::ConsoleColorFlags::Red | util::ConsoleColorFlags::Intensity,Color{209,17,65}},
	{util::ConsoleColorFlags::Green | util::ConsoleColorFlags::Intensity,Color{0,177,89}},
	{util::ConsoleColorFlags::Blue | util::ConsoleColorFlags::Intensity,Color{0,174,219}},
	{util::ConsoleColorFlags::Yellow | util::ConsoleColorFlags::Intensity,Color{255,196,37}},
	{util::ConsoleColorFlags::Cyan | util::ConsoleColorFlags::Intensity,Color{132,193,255}},
	{util::ConsoleColorFlags::Magenta | util::ConsoleColorFlags::Intensity,Color{255,51,119}},
	{util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity,Color{255,255,255}},

	{util::ConsoleColorFlags::Red,Color{209 /2,17 /2,65 /2}},
	{util::ConsoleColorFlags::Green,Color{0 /2,177 /2,89 /2}},
	{util::ConsoleColorFlags::Blue,Color{0 /2,174 /2,219 /2}},
	{util::ConsoleColorFlags::Yellow,Color{255 /2,196 /2,37 /2}},
	{util::ConsoleColorFlags::Cyan,Color{132 /2,193 /2,255 /2}},
	{util::ConsoleColorFlags::Magenta,Color{255 /2,51 /2,119 /2}},
	{util::ConsoleColorFlags::White,Color{255 /2,255 /2,255 /2}}
};
std::optional<Color> util::console_color_flags_to_color(ConsoleColorFlags flags)
{
	auto foregroundFlags = flags &(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	auto it = colorMap.find(foregroundFlags);
	if(it != colorMap.end())
		return it->second;
	return {};
}

util::ConsoleColorFlags util::color_to_console_color_flags(const Color &color)
{
	auto smallestDiff = std::numeric_limits<uint16_t>::max();
	auto bestCandidate = util::ConsoleColorFlags::None;
	for(auto &pair : colorMap)
	{
		auto &colOther = pair.second;
		auto diff = static_cast<uint16_t>(umath::abs(color.r -colOther.r) +umath::abs(color.g -colOther.g) +umath::abs(color.b -colOther.b));
		if(diff > smallestDiff)
			continue;
		smallestDiff = diff;
		bestCandidate = pair.first;
	}
	return bestCandidate;
}
