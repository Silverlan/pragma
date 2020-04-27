/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __UTIL_WINDING_ORDER_HPP__
#define __UTIL_WINDING_ORDER_HPP__

#include <cinttypes>

enum class WindingOrder : uint8_t
{
	Clockwise = 0u,
	CounterClockwise
};

#endif
