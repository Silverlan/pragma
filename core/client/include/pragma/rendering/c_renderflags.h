// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_RENDERFLAGS_H__
#define __C_RENDERFLAGS_H__

#include <mathutil/umath.h>

enum class RenderFlags : uint32_t {
	None = 0,
	World = 1,
	View = World << 1,
	Skybox = View << 1,
	Shadows = Skybox << 1,
	Particles = Shadows << 1,
	Debug = Particles << 1,
	Water = Debug << 1,
	Static = Water << 1,
	Dynamic = Static << 1,
	Translucent = Dynamic << 1,

	All = (Translucent << 1) - 1,
	Reflection = Translucent << 1,
	HDR = Reflection << 1,
	ParticleDepth = HDR << 1
};
REGISTER_BASIC_BITWISE_OPERATORS(RenderFlags)

#endif
