// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PRAGMA_COMPONENT_MEMBER_FLAGS_HPP__
#define __PRAGMA_COMPONENT_MEMBER_FLAGS_HPP__

#include <cinttypes>
#include <mathutil/umath.h>

namespace pragma {
	enum class ComponentMemberFlags : uint32_t {
		None = 0,
		HideInInterface = 1,
		Controller = HideInInterface << 1u,
		WorldSpace = Controller << 1u,
		ObjectSpace = WorldSpace << 1u,
		// This is merely a hint flag for interfaces not to allow writing this property
		ReadOnly = ObjectSpace << 1u
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ComponentMemberFlags)

#endif
