// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_LWORLDENVIRONMENT_HPP__
#define __C_LWORLDENVIRONMENT_HPP__

#include "pragma/definitions.h"
#include "pragma/rendering/world_environment.hpp"
#include <pragma/lua/ldefinitions.h>

namespace Lua {
	namespace WorldEnvironment {
		DLLCLIENT void register_class(luabind::class_<::WorldEnvironment> &classDef);
	};
};

#endif
