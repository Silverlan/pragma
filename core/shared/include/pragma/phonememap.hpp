/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PHONEMEMAP_HPP__
#define __PHONEMEMAP_HPP__

#include "pragma/networkdefinitions.h"
#include <string>
#include <unordered_map>

struct DLLNETWORK PhonemeInfo {
	std::unordered_map<std::string, float> flexControllers;
	bool operator==(const PhonemeInfo &other) const
	{
		if(flexControllers.size() != other.flexControllers.size())
			return false;
		for(auto &pair : flexControllers) {
			auto it = other.flexControllers.find(pair.first);
			if(it == other.flexControllers.end())
				return false;
			if(umath::abs(pair.second - it->second) > 0.001f)
				return false;
		}
		return true;
	}
	bool operator!=(const PhonemeInfo &other) const { return !operator==(other); }
};

struct DLLNETWORK PhonemeMap {
	std::unordered_map<std::string, PhonemeInfo> phonemes;
	bool operator==(const PhonemeMap &other) const { return phonemes == other.phonemes; }
	bool operator!=(const PhonemeMap &other) const { return !operator==(other); }
};

#endif
