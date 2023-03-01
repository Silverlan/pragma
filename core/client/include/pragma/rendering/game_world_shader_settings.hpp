/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __GAME_WORLD_SHADER_SETTINGS_HPP__
#define __GAME_WORLD_SHADER_SETTINGS_HPP__

#include "pragma/c_enginedefinitions.h"
#include "pragma/clientdefinitions.h"
#include <cinttypes>

namespace pragma {
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

#endif
