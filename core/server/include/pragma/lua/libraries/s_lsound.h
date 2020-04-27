/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_LSOUND_H__
#define __S_LSOUND_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include <pragma/lua/libraries/lsound.h>

namespace Lua
{
	namespace sound
	{
		namespace Server
		{
			DLLSERVER int create(lua_State *l);
		};
	};
};

#endif

