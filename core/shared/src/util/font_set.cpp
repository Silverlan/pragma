/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/util/font_set.hpp"

bool FontSet::HasFeature(const std::string_view &feature) const { return std::find(features.begin(), features.end(), feature) != features.end(); }
static uint32_t get_number_of_bits_set(FontSetFlag flags)
{
	auto numBytes = umath::to_integral(flags);
	auto numBits = numBytes * 8;
	for(auto i = decltype(numBits) {0u}; i < numBits; ++i) {
		if((umath::to_integral(flags) & (i + 1)) != 0)
			++numBits;
	}
	return numBits;
}
FontSetFileData *FontSet::FindFontFileCandidate(FontSetFlag flags)
{
	FontSetFileData *bestCandidate = nullptr;
	uint32_t bestCandidateSetBits = 0;
	for(auto &fd : fileData) {
		if(fd.flags == flags)
			return &fd;
		auto andFlags = fd.flags & flags;
		auto numBitsSet = get_number_of_bits_set(andFlags);
		if(!bestCandidate || numBitsSet > bestCandidateSetBits) {
			bestCandidate = &fd;
			bestCandidateSetBits = numBitsSet;
		}
	}
	return bestCandidate;
}
const FontSetFileData *FontSet::FindFontFileCandidate(FontSetFlag flags) const { return const_cast<FontSet *>(this)->FindFontFileCandidate(flags); }
