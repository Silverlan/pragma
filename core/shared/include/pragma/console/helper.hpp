// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PRAGMA_CONSOLE_HELPER_HPP__
#define __PRAGMA_CONSOLE_HELPER_HPP__

#define UVN_PASTE(a, b) a##b
#define UVN_EVAL(a, b) UVN_PASTE(a, b)

// Unique variable name
#define UVN UVN_EVAL(u, __COUNTER__)

#endif
