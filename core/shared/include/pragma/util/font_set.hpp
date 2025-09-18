// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PRAGMA_FONT_SET_HPP__
#define __PRAGMA_FONT_SET_HPP__

#include "pragma/networkdefinitions.h"
#include <cinttypes>
#include <string>
#include <optional>
#include <string_view>
#include <vector>

enum class FontSetFlag : uint8_t {
	None = 0,
	Bold = 1,
	Italic = Bold << 1u,
	Mono = Italic << 1u,
	Serif = Mono << 1u,
	Sans = Serif << 1u,
};
struct DLLNETWORK FontSetFileData {
	FontSetFlag flags = FontSetFlag::None;
	std::string fileName;
	std::optional<uint32_t> fontSizeAdjustment {};
};
struct DLLNETWORK FontSet {
	std::vector<FontSetFileData> fileData;
	std::vector<std::string> features;
	FontSetFileData *FindFontFileCandidate(FontSetFlag flags);
	const FontSetFileData *FindFontFileCandidate(FontSetFlag flags) const;
	bool HasFeature(const std::string_view &feature) const;
};
REGISTER_BASIC_BITWISE_OPERATORS(FontSetFlag)

#endif
