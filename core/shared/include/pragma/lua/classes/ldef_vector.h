/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LDEF_VECTOR_H__
#define __LDEF_VECTOR_H__
#include "pragma/lua/ldefinitions.h"
#include <mathutil/glmutil.h>
lua_registercheck(Vector, ::Vector3);
lua_registercheck(Vectori, ::Vector3i);
lua_registercheck(Vector2, ::Vector2);
lua_registercheck(Vector2i, ::Vector2i);
lua_registercheck(Vector4, ::Vector4);
lua_registercheck(Vector4i, ::Vector4i);
#endif
