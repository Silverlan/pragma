// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/rendering/c_alpha_mode.hpp"

bool pragma::premultiply_alpha(Vector4 &color, pragma::ParticleAlphaMode alphaMode)
{
	switch(alphaMode) {
	case pragma::ParticleAlphaMode::Opaque:
		color = {color.r * color.a, color.g * color.a, color.b * color.a, 1.f};
		break;
	case pragma::ParticleAlphaMode::Translucent:
		color = {color.r * color.a, color.g * color.a, color.b * color.a, color.a};
		break;
	case pragma::ParticleAlphaMode::Additive:
	case pragma::ParticleAlphaMode::AdditiveByColor:
		color = {color.r * color.a, color.g * color.a, color.b * color.a, 0.f};
		break;
	case pragma::ParticleAlphaMode::Premultiplied:
		// No change
		break;
	default:
		return false;
	}
	return true;
}
