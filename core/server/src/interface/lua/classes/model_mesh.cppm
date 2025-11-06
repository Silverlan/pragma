// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.server:scripting.lua.classes.model_mesh;

export namespace Lua {
	namespace ModelMesh {
		namespace Server {
			DLLSERVER void Create(lua::State *l);
		};
	};
	namespace ModelSubMesh {
		namespace Server {
			DLLSERVER void Create(lua::State *l);
		};
	};
};
