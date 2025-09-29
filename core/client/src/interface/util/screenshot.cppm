// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <util_image.hpp>

export module pragma.client:util.screenshot;
import :game;

export namespace util {
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
