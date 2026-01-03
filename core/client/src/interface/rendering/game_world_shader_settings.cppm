// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.game_world_shader_settings;

export import std.compat;

export namespace pragma {
	namespace rendering {
		struct DLLCLIENT GameWorldShaderSettings {
			enum class ShadowQuality : uint32_t { VeryLow = 0, Low, Medium, High, VeryHigh };
			// TODO: Enable these once C++20 is available
			//bool operator==(const GameWorldShaderSettings&) const=default;
			//bool operator!=(const GameWorldShaderSettings&) const=default;
			bool operator==(const GameWorldShaderSettings &other) const
			{
				return other.shadowQuality == shadowQuality && other.ssaoEnabled == ssaoEnabled && other.bloomEnabled == bloomEnabled && other.fxaaEnabled == fxaaEnabled && other.debugModeEnabled == debugModeEnabled && other.iblEnabled == iblEnabled
				  && other.dynamicLightingEnabled == dynamicLightingEnabled && other.dynamicShadowsEnabled == dynamicShadowsEnabled;
				static_assert(sizeof(GameWorldShaderSettings) == 12);
			}
			bool operator!=(const GameWorldShaderSettings &other) const { return !operator==(other); }
			ShadowQuality shadowQuality = ShadowQuality::Medium;
			bool ssaoEnabled = true;
			bool bloomEnabled = true;
			bool fxaaEnabled = true;
			bool debugModeEnabled = false;
			bool iblEnabled = true;
			bool dynamicLightingEnabled = true;
			bool dynamicShadowsEnabled = true;
		};
	};
};
