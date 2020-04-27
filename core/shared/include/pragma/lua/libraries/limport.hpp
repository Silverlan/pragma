/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __LIMPORT_HPP__
#define __LIMPORT_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/ldefinitions.h"

struct aiScene;
class Model;
struct aiScene;
namespace pragma {class SceneSnapshot;};
namespace Lua
{
	namespace import
	{
		DLLNETWORK int import_wrci(lua_State *l);
		DLLNETWORK int import_wad(lua_State *l);
		DLLNETWORK int import_wrmi(lua_State *l);
		DLLNETWORK int import_smd(lua_State *l);
		DLLNETWORK int import_obj(lua_State *l);
		DLLNETWORK int import_model_asset(lua_State *l);

		DLLNETWORK std::shared_ptr<aiScene> snapshot_to_assimp_scene(const pragma::SceneSnapshot &snapshot);
		DLLNETWORK int export_model_asset(lua_State *l);
	};
};

#endif
