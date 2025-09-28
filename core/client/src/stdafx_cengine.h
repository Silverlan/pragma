// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifdef _WIN32
#pragma message("Compiling precompiled headers.\n")
#endif
#ifdef _WIN32
#define _WIN32_WINNT 0x0501
#include <Windows.h>
#endif
#include <mathutil/glmutil.h>
#include <mathutil/eulerangles.h>
#include <mathutil/uquat.h>
#include <mathutil/uvec.h>
#include <sharedutils/util.h>
#include <sharedutils/util_string.h>
#include <mathutil/umath.h>
#include <algorithm>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
