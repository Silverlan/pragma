#include "stdafx_client.h"
#include "pragma/rendering/c_alpha_mode.hpp"

bool pragma::premultiply_alpha(Vector4 &color,pragma::AlphaMode alphaMode)
{
	switch(alphaMode)
	{
		case pragma::AlphaMode::Opaque:
			color = {color.r *color.a,color.g *color.a,color.b *color.a,1.f};
			break;
		case pragma::AlphaMode::Translucent:
			color = {color.r *color.a,color.g *color.a,color.b *color.a,color.a};
			break;
		case pragma::AlphaMode::Additive:
		case pragma::AlphaMode::AdditiveFull:
			color = {color.r *color.a,color.g *color.a,color.b *color.a,0.f};
			break;
		case pragma::AlphaMode::Premultiplied:
			// No change
			break;
		default:
			return false;
	}
	return true;
}
