/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __DEBUG_LUA_ZEROBRANE_HPP__
#define __DEBUG_LUA_ZEROBRANE_HPP__

#include "pragma/networkdefinitions.h"
#include <string>
#include <cinttypes>

namespace debug
{
	DLLNETWORK void open_file_in_zerobrane(const std::string &fileName,uint32_t lineIdx);
};

#endif
