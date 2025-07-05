// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LDEF_MAT4_H__
#define __LDEF_MAT4_H__
#include "pragma/lua/ldefinitions.h"
#include <mathutil/glmutil.h>

lua_registercheck(Mat2, ::Mat2);
lua_registercheck(Mat2x3, ::Mat2x3);
lua_registercheck(Mat2x4, ::Mat2x4);
lua_registercheck(Mat3, ::Mat3);
lua_registercheck(Mat3x2, ::Mat3x2);
lua_registercheck(Mat3x4, ::Mat3x4);
lua_registercheck(Mat4, ::Mat4);
lua_registercheck(Mat4x2, ::Mat4x2);
lua_registercheck(Mat4x3, ::Mat4x3);
#endif
