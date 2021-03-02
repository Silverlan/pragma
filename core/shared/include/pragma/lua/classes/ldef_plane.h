/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LDEF_PLANE_H__
#define __LDEF_PLANE_H__
#include "pragma/lua/ldefinitions.h"
#include <mathutil/plane.hpp>
lua_registercheck(Plane,umath::Plane);
#endif