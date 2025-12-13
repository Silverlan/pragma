// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.physics;

export import pragma.lua;

export {
	namespace Lua {
		namespace physenv {
			DLLNETWORK void register_library(Interface &lua);
		};
		namespace PhysShape {
			DLLNETWORK void register_class(lua::State *l, luabind::module_ &mod);
		};
		namespace PhysCollisionObj {
			DLLNETWORK void register_class(lua::State *l, luabind::module_ &mod);
		};
		namespace PhysContact {
			DLLNETWORK void register_class(lua::State *l, luabind::module_ &mod);
		};
	};

	namespace Lua {
		namespace PhysConstraint {
			DLLNETWORK void register_class(lua::State *l, luabind::module_ &mod);
		};
	};
};
