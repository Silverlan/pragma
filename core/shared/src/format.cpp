// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/string/format.h"
#include <cmath>
std::string FormatTime(double dtm)
{
	int tm = static_cast<int>(floor(dtm + 0.5));
	int minutes = tm / 60;
	int seconds = tm % 60;
	std::stringstream ss;
	if(minutes < 10)
		ss << "0";
	ss << minutes << ":";
	if(seconds < 10)
		ss << "0";
	ss << seconds;
	return ss.str();
}
