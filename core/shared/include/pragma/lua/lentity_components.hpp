/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LENTITY_COMPONENTS_HPP__
#define __LENTITY_COMPONENTS_HPP__

#include "pragma/networkdefinitions.h"

struct lua_State;
namespace luabind {class module_;};
struct BaseSoundEmitterComponentHandleWrapper;
namespace Lua
{
	DLLNETWORK void register_gravity_component(luabind::module_ &module);
};

#endif
