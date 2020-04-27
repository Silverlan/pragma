/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_LIMPORT_HPP__
#define __C_LIMPORT_HPP__

#include "pragma/clientdefinitions.h"

struct lua_State;
namespace Lua
{
	namespace lib_export
	{
		DLLCLIENT int export_scene(lua_State *l);
	};
};

#endif
