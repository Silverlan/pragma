/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

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
