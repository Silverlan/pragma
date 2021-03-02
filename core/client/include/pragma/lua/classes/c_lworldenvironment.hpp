/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LWORLDENVIRONMENT_HPP__
#define __C_LWORLDENVIRONMENT_HPP__

#include "pragma/definitions.h"
#include "pragma/rendering/world_environment.hpp"
#include <pragma/lua/ldefinitions.h>

namespace Lua
{
	namespace WorldEnvironment
	{
		DLLCLIENT void register_class(luabind::class_<::WorldEnvironment> &classDef);
	};
};

#endif
