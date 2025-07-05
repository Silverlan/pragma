// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_SETTINGS_HPP__
#define __C_SETTINGS_HPP__

namespace pragma::rendering {
	enum class AntiAliasing : uint8_t { None = 0u, MSAA, FXAA };

	enum class ToneMapping : uint32_t { None = 0u, GammaCorrection, Reinhard, HejilRichard, Uncharted, Aces, GranTurismo };
};

#endif
