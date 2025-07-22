// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PRAGMA_CONSOLE_LINENOISE_HPP__
#define __PRAGMA_CONSOLE_LINENOISE_HPP__

#include "pragma/networkdefinitions.h"
#ifdef __linux__

namespace pragma::console::impl {
    void init_linenoise();
    void update_linenoise();
};

#endif
#endif
