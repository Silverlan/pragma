// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <mathutil/umath.h>

export module pragma.shared:assets.vtf_info;

export {
	namespace pragma::asset {
		struct DLLNETWORK VtfInfo {
			enum class Format : uint32_t {
				Bc1 = 0,
				Bc1a,
				Bc2,
				Bc3,
				R8G8B8A8_UNorm,
				R8G8_UNorm,
				R16G16B16A16_SFloat,
				R32G32B32A32_SFloat,
				A8B8G8R8_UNorm_Pack32,
				B8G8R8A8_UNorm,

				Count
			};
			enum class Flags : uint32_t { None = 0u, Srgb = 1u, NormalMap = Srgb << 1u, GenerateMipmaps = NormalMap << 1u };
			Format inputFormat = Format::R8G8B8A8_UNorm;
			Format outputFormat = Format::Bc1;
			Flags flags = Flags::None;
		};
	};
	REGISTER_BASIC_BITWISE_OPERATORS(pragma::asset::VtfInfo::Flags);
};
