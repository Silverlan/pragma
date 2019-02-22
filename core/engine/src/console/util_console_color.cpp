#include "stdafx_engine.h"
#include "pragma/console/util_console_color.hpp"

bool util::set_console_color(ConsoleColorFlags flags)
{
	reset_console_color();
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
}
bool util::reset_console_color()
{
	std::cout.flush();
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

