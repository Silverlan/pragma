/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_engine.h"
#include "pragma/string/format.h"
#include <cmath>
std::string FormatTime(double dtm)
{
	int tm = static_cast<int>(floor(dtm +0.5));
	int minutes = tm /60;
	int seconds = tm %60;
	std::stringstream ss;
	if(minutes < 10) ss<<"0";
	ss<<minutes<<":";
	if(seconds < 10) ss<<"0";
	ss<<seconds;
	return ss.str();
}