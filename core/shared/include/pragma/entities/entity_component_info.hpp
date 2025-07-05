// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __ENTITY_COMPONENT_INFO_HPP__
#define __ENTITY_COMPONENT_INFO_HPP__

#include <cinttypes>
#include <limits>

namespace pragma {
	using ComponentId = uint32_t;
	using ComponentEventId = uint32_t;
	const auto INVALID_COMPONENT_ID = std::numeric_limits<ComponentId>::max();
};

#endif
