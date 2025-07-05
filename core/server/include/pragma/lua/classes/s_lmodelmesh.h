// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_LMODELMESH_H__
#define __S_LMODELMESH_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>

class ModelMesh;

namespace Lua {
	namespace ModelMesh {
		namespace Server {
			DLLSERVER void Create(lua_State *l);
		};
	};
	namespace ModelSubMesh {
		namespace Server {
			DLLSERVER void Create(lua_State *l);
		};
	};
};

#endif
