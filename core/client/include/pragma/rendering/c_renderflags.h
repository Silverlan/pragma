/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_RENDERFLAGS_H__
#define __C_RENDERFLAGS_H__

#include <mathutil/umath.h>

enum class FRender : uint32_t
{
	None = 0,
	World = 1,
	View = World<<1,
	Skybox = View<<1,
	Shadows = Skybox<<1,
	Particles = Shadows<<1,
	Glow = Particles<<1,
	Debug = Glow<<1,
	Water = Debug<<1,
	Static = Water<<1,
	Dynamic = Static<<1,
	All = (Dynamic <<1) -1,
	Reflection = Dynamic<<1,
	HDR = Reflection<<1,
	ParticleDepth = HDR<<1
};
REGISTER_BASIC_BITWISE_OPERATORS(FRender)

#endif