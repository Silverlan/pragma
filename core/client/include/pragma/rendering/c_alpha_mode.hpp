#ifndef __C_ALPHA_MODE_HPP__
#define __C_ALPHA_MODE_HPP__

#include "pragma/clientdefinitions.h"
#include <cinttypes>
#include <mathutil/uvec.h>

namespace pragma
{
	enum class AlphaMode : uint32_t
	{
		Additive = 0u,
		AdditiveFull,
		Opaque,
		Masked,
		Translucent,
		Premultiplied,

		Count
	};
	DLLCLIENT bool premultiply_alpha(Vector4 &color,pragma::AlphaMode alphaMode);
};

#endif
