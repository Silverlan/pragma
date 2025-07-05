// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_ALPHA_MODE_HPP__
#define __C_ALPHA_MODE_HPP__

#include "pragma/clientdefinitions.h"
#include <sharedutils/util_pragma.hpp>
#include <cinttypes>
#include <mathutil/uvec.h>

namespace pragma {
	enum class ParticleAlphaMode : uint32_t {
		Additive = 0u,
		AdditiveByColor,
		Opaque,
		Masked,
		Translucent,
		Premultiplied,
		Custom,

		Count
	};
	DLLCLIENT bool premultiply_alpha(Vector4 &color, pragma::ParticleAlphaMode alphaMode);
};

#endif
