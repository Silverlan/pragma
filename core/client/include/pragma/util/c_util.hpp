/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __PRAGMA_C_UTIL_HPP__
#define __PRAGMA_C_UTIL_HPP__

#include "pragma/clientdefinitions.h"
#include <util_image.hpp>

class CGame;
namespace util {
	struct RtScreenshotSettings {
		bool denoise = true;
		float quality = 1.f;
		uint32_t samples = 1024;
		uimg::ToneMapping toneMapping = uimg::ToneMapping::GammaCorrection;
		std::optional<std::string> sky;
		float skyStrength = 1.f;
		EulerAngles skyAngles {};
	};
	DLLCLIENT void rt_screenshot(CGame &game, uint32_t width, uint32_t height, const RtScreenshotSettings &settings, uimg::ImageFormat format = uimg::ImageFormat::PNG);
	DLLCLIENT std::optional<std::string> screenshot(CGame &game);
};

#endif
