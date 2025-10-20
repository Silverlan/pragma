// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <cinttypes>
#include <string>
#include <optional>
#include <string_view>
#include <vector>

export module pragma.shared:util.font_set;

import pragma.math;

export {
	namespace pragma {
		enum class FontSetFlag : uint8_t {
			None = 0,
			Bold = 1,
			Italic = Bold << 1u,
			Mono = Italic << 1u,
			Serif = Mono << 1u,
			Sans = Serif << 1u,
		};
        using namespace umath::scoped_enum::bitwise;
	}
    namespace umath::scoped_enum::bitwise {
        template<>
        struct enable_bitwise_operators<pragma::FontSetFlag> : std::true_type {};
    }

	struct DLLNETWORK FontSetFileData {
		pragma::FontSetFlag flags = pragma::FontSetFlag::None;
		std::string fileName;
		std::optional<uint32_t> fontSizeAdjustment {};
	};
	struct DLLNETWORK FontSet {
		std::vector<FontSetFileData> fileData;
		std::vector<std::string> features;
		FontSetFileData *FindFontFileCandidate(pragma::FontSetFlag flags);
		const FontSetFileData *FindFontFileCandidate(pragma::FontSetFlag flags) const;
		bool HasFeature(const std::string_view &feature) const;
	};
};
