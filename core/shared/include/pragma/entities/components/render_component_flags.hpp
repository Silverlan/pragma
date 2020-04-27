/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __RENDER_COMPONENT_FLAGS_HPP__
#define __RENDER_COMPONENT_FLAGS_HPP__

#include <mathutil/umath.h>

namespace pragma
{
	enum class FRenderFlags : uint32_t
	{
		None = 0u,
		CastShadows = 1u,
		Unlit = CastShadows<<1u
	};
	REGISTER_BASIC_BITWISE_OPERATORS(FRenderFlags);
};

#endif
