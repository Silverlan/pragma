/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ENTITY_COMPONENT_INFO_HPP__
#define __ENTITY_COMPONENT_INFO_HPP__

#include <cinttypes>

namespace pragma {
	using ComponentId = uint32_t;
	using ComponentEventId = uint32_t;
	const auto INVALID_COMPONENT_ID = std::numeric_limits<ComponentId>::max();
};

#endif
