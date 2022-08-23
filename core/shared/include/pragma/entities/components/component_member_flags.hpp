/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PRAGMA_COMPONENT_MEMBER_FLAGS_HPP__
#define __PRAGMA_COMPONENT_MEMBER_FLAGS_HPP__

#include <cinttypes>

namespace pragma
{
	enum class ComponentMemberFlags : uint32_t
	{
		None = 0,
		HideInInterface = 1,
		Controller = HideInInterface<<1u
	};
};

#endif
