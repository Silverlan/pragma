/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LALSOUND_H__
#define __LALSOUND_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include <mathutil/glmutil.h>

struct lua_State;
class ALSound;
namespace Lua
{
	namespace ALSound
	{
		DLLNETWORK void register_class(luabind::class_<::ALSound> &classDef);
	};
};
std::ostream &operator<<(std::ostream &out,const ::ALSound &snd);

#endif
