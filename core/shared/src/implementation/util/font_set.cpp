// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :util.font_set;

bool FontSet::HasFeature(const std::string_view &feature) const { return std::find(features.begin(), features.end(), feature) != features.end(); }
static uint32_t get_number_of_bits_set(pragma::FontSetFlag flags)
{
	auto numBytes = pragma::math::to_integral(flags);
	auto numBits = numBytes * 8;
	for(auto i = decltype(numBits) {0u}; i < numBits; ++i) {
		if((pragma::math::to_integral(flags) & (i + 1)) != 0)
			++numBits;
	}
	return numBits;
}
FontSetFileData *FontSet::FindFontFileCandidate(pragma::FontSetFlag flags)
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
const FontSetFileData *FontSet::FindFontFileCandidate(pragma::FontSetFlag flags) const { return const_cast<FontSet *>(this)->FindFontFileCandidate(flags); }
