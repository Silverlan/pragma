/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PRAGMA_LUA_ENTITY_COMPONENTS_HPP__
#define __PRAGMA_LUA_ENTITY_COMPONENTS_HPP__

#include "pragma/lua/luaapi.h"

namespace pragma {
	class BaseAttachableComponent;
	class BaseAnimatedComponent;
};
namespace pragma::lua
{
	DLLNETWORK void register_entity_component_classes(luabind::module_ &mod);
};

#endif
