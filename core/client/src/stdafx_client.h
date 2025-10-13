// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifdef _WIN32
// #pragma message("Compiling precompiled headers.\n")
#define _WIN32_WINNT 0x0501
#define NOMINMAX
#include <Windows.h>
#endif
#ifdef _WIN32
#endif
#include <algorithm>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#ifdef __linux__
#endif
