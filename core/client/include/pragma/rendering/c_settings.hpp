/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SETTINGS_HPP__
#define __C_SETTINGS_HPP__

namespace pragma::rendering
{
	enum class AntiAliasing : uint8_t
	{
		None = 0u,
		MSAA,
		FXAA
	};

	enum class ToneMapping : uint32_t
	{
		None = 0u,
		GammaCorrection,
		Reinhard,
		HejilRichard,
		Uncharted,
		Aces,
		GranTurismo
	};
};

#endif
