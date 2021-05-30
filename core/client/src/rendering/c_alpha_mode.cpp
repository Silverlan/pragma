/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/c_alpha_mode.hpp"

bool pragma::premultiply_alpha(Vector4 &color,pragma::ParticleAlphaMode alphaMode)
{
	switch(alphaMode)
	{
		case pragma::ParticleAlphaMode::Opaque:
			color = {color.r *color.a,color.g *color.a,color.b *color.a,1.f};
			break;
		case pragma::ParticleAlphaMode::Translucent:
			color = {color.r *color.a,color.g *color.a,color.b *color.a,color.a};
			break;
		case pragma::ParticleAlphaMode::Additive:
		case pragma::ParticleAlphaMode::AdditiveByColor:
			color = {color.r *color.a,color.g *color.a,color.b *color.a,0.f};
			break;
		case pragma::ParticleAlphaMode::Premultiplied:
			// No change
			break;
		default:
			return false;
	}
	return true;
}
