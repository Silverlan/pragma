/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADOW_TYPE_HPP__
#define __C_SHADOW_TYPE_HPP__

#include <cinttypes>
#include <mathutil/umath.h>

enum class ShadowType : uint8_t { None = 0, StaticOnly = 1, Full = StaticOnly | 2 };
REGISTER_BASIC_ARITHMETIC_OPERATORS(ShadowType);

#endif
