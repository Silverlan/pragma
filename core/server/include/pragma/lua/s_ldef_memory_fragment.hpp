/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_LDEF_MEMORY_FRAGMENT_HPP__
#define __S_LDEF_MEMORY_FRAGMENT_HPP__

#include <pragma/lua/ldefinitions.h>
#include "pragma/ai/ai_memory.h"

lua_registercheck(AIMemoryFragment,pragma::ai::Memory::Fragment);

#endif
