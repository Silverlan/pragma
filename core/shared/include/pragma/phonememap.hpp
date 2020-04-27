/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __PHONEMEMAP_HPP__
#define __PHONEMEMAP_HPP__

#include "pragma/networkdefinitions.h"
#include <string>
#include <unordered_map>

struct DLLNETWORK PhonemeInfo
{
	std::unordered_map<std::string,float> flexControllers;
};

struct DLLNETWORK PhonemeMap
{
	std::unordered_map<std::string,PhonemeInfo> phonemes;
};

#endif
