// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_LIMPORT_HPP__
#define __C_LIMPORT_HPP__

#include "pragma/clientdefinitions.h"

struct lua_State;
namespace Lua {
	namespace lib_export {
		DLLCLIENT int export_scene(lua_State *l);
	};
};

#endif
