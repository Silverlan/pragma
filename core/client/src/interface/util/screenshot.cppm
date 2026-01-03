// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:util.screenshot;

export import :game;

export namespace pragma::util {
	struct RtScreenshotSettings {
		bool denoise = true;
		float quality = 1.f;
		uint32_t samples = 1024;
		image::ToneMapping toneMapping = image::ToneMapping::GammaCorrection;
		std::optional<std::string> sky;
		float skyStrength = 1.f;
		EulerAngles skyAngles {};
	};
	DLLCLIENT void rt_screenshot(CGame &game, uint32_t width, uint32_t height, const RtScreenshotSettings &settings, image::ImageFormat format = image::ImageFormat::PNG);
	DLLCLIENT std::optional<std::string> screenshot(CGame &game);
};
